#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
#include <visualization_msgs/Marker.h>
#include <algorithm>
#include <cmath>
#include <tf/tf.h>

/**
 * This class implements a simple autonomous exploration behavior.
 * The robot moves toward open spaces while avoiding obstacles.
 */
class AutonomousExplorer
{
private:
    // ROS communication objects
    ros::NodeHandle nh_;
    ros::Subscriber scan_sub_;
    ros::Publisher cmd_pub_;
    ros::Publisher marker_pub_;
    
    // Behavior parameters (can be set via ROS parameters)
    double forward_speed_;
    double rotation_speed_;
    double min_safe_distance_;
    double max_scan_angle_;  // How much of the scan to consider (in radians)
    
    // Latest scan data
    sensor_msgs::LaserScan latest_scan_;
    bool scan_received_;
    
public:
    AutonomousExplorer() : scan_received_(false)
    {	
	// Create private node handle for private 
	ros::NodeHandle private_nh("~");
        // Subscribe to laser scan data
        scan_sub_ = nh_.subscribe("/scan", 1, &AutonomousExplorer::scanCallback, this);
        
        // Publish velocity commands
        cmd_pub_ = nh_.advertise<geometry_msgs::Twist>("/cmd_vel", 1);
        
        // Publish visualization markers
        marker_pub_ = nh_.advertise<visualization_msgs::Marker>("/exploration_markers", 10);
        
        // Load parameters with defaults
        // The ~ means this is a private parameter (scoped to this node)
        nh_.param("forward_speed", forward_speed_, 0.3);
        nh_.param("rotation_speed", rotation_speed_, 0.8);
        nh_.param("min_safe_distance", min_safe_distance_, 0.5);
        nh_.param("max_scan_angle", max_scan_angle_, M_PI / 2.0);  // 90 degrees on each side
        
        ROS_INFO("Autonomous Explorer initialized");
        ROS_INFO("Forward speed: %.2f m/s", forward_speed_);
        ROS_INFO("Rotation speed: %.2f rad/s", rotation_speed_);
        ROS_INFO("Min safe distance: %.2f m", min_safe_distance_);
        ROS_INFO("Scan angle range: %.2f degrees", max_scan_angle_ * 180.0 / M_PI * 2);
    }
    
    /**
     * Callback for laser scan messages.
     * This is where the main decision-making happens.
     */
    void scanCallback(const sensor_msgs::LaserScan::ConstPtr& scan)
    {
        latest_scan_ = *scan;
        scan_received_ = true;
        
        // Process the scan and make movement decisions
        processAndMove();
    }
    
    /**
     * Determines which scan indices correspond to the front of the robot
     */
    void getFrontScanIndices(int& start_idx, int& end_idx)
    {
        // Calculate how many measurements fit in our desired angle range
        int measurements_in_range = static_cast<int>(max_scan_angle_ / latest_scan_.angle_increment);
        
        // Get the center index (forward direction)
        int center_idx = latest_scan_.ranges.size() / 2;
        
        // Calculate start and end indices
        start_idx = std::max(0, center_idx - measurements_in_range);
        end_idx = std::min(static_cast<int>(latest_scan_.ranges.size()) - 1, 
                          center_idx + measurements_in_range);
    }
    
    /**
     * Finds the minimum and maximum valid ranges in the front scan area
     */
    void findMinMaxRanges(double& min_range, double& max_range, 
                         double& min_angle, double& max_angle)
    {
        int start_idx, end_idx;
        getFrontScanIndices(start_idx, end_idx);
        
        min_range = latest_scan_.range_max;
        max_range = latest_scan_.range_min;
        min_angle = 0.0;
        max_angle = 0.0;
        
        // Scan through the front measurements
        for (int i = start_idx; i <= end_idx; ++i)
        {
            float range = latest_scan_.ranges[i];
            
            // Only consider valid measurements
            if (range >= latest_scan_.range_min && range <= latest_scan_.range_max)
            {
                double angle = latest_scan_.angle_min + i * latest_scan_.angle_increment;
                
                if (range < min_range)
                {
                    min_range = range;
                    min_angle = angle;
                }
                
                if (range > max_range)
                {
                    max_range = range;
                    max_angle = angle;
                }
            }
        }
    }
    
    /**
     * Creates a marker showing the scan range being considered
     */
    visualization_msgs::Marker createScanRangeMarker()
    {
        visualization_msgs::Marker marker;
        marker.header.frame_id = "base_footprint";
        marker.header.stamp = ros::Time::now();
        marker.ns = "scan_range";
        marker.id = 0;
        marker.type = visualization_msgs::Marker::LINE_STRIP;
        marker.action = visualization_msgs::Marker::ADD;
        
        marker.scale.x = 0.02;  // Line width
        marker.color.r = 1.0;
        marker.color.g = 1.0;
        marker.color.b = 0.0;
        marker.color.a = 0.5;
        
        int start_idx, end_idx;
        getFrontScanIndices(start_idx, end_idx);
        
        // Add points along the scan range
        for (int i = start_idx; i <= end_idx; i += 10)  // Skip some for performance
        {
            if (latest_scan_.ranges[i] >= latest_scan_.range_min && 
                latest_scan_.ranges[i] <= latest_scan_.range_max)
            {
                double angle = latest_scan_.angle_min + i * latest_scan_.angle_increment;
                geometry_msgs::Point p;
                p.x = latest_scan_.ranges[i] * cos(angle);
                p.y = latest_scan_.ranges[i] * sin(angle);
                p.z = 0.0;
                marker.points.push_back(p);
            }
        }
        
        return marker;
    }
    
    /**
     * Creates a sphere marker at a specific angle and distance
     */
    visualization_msgs::Marker createPointMarker(int id, double range, double angle, 
                                                 float r, float g, float b, std::string ns)
    {
        visualization_msgs::Marker marker;
        marker.header.frame_id = "base_footprint";
        marker.header.stamp = ros::Time::now();
        marker.ns = ns;
        marker.id = id;
        marker.type = visualization_msgs::Marker::SPHERE;
        marker.action = visualization_msgs::Marker::ADD;
        
        marker.pose.position.x = range * cos(angle);
        marker.pose.position.y = range * sin(angle);
        marker.pose.position.z = 0.2;
        marker.pose.orientation.w = 1.0;
        
        marker.scale.x = 0.2;
        marker.scale.y = 0.2;
        marker.scale.z = 0.2;
        
        marker.color.r = r;
        marker.color.g = g;
        marker.color.b = b;
        marker.color.a = 1.0;
        
        return marker;
    }
    
    /**
     * Creates an arrow marker showing the intended direction
     */
    visualization_msgs::Marker createDirectionArrow(double angle)
    {
        visualization_msgs::Marker marker;
        marker.header.frame_id = "base_footprint";
        marker.header.stamp = ros::Time::now();
        marker.ns = "target_direction";
        marker.id = 3;
        marker.type = visualization_msgs::Marker::ARROW;
        marker.action = visualization_msgs::Marker::ADD;
        
        marker.scale.x = 0.5;  // Arrow length
        marker.scale.y = 0.05;  // Arrow width
        marker.scale.z = 0.05;  // Arrow height
        
        marker.color.r = 0.0;
        marker.color.g = 1.0;
        marker.color.b = 0.0;
        marker.color.a = 1.0;
        
        // Arrow points along +X by default, so we rotate it
        marker.pose.position.x = 0.0;
        marker.pose.position.y = 0.0;
        marker.pose.position.z = 0.3;
        
        // Convert angle to quaternion
        tf::Quaternion q;
        q.setRPY(0, 0, angle);
        marker.pose.orientation.x = q.x();
        marker.pose.orientation.y = q.y();
        marker.pose.orientation.z = q.z();
        marker.pose.orientation.w = q.w();
        
        return marker;
    }
    
    /**
     * Main processing function: analyzes scan and publishes movement commands
     */
    void processAndMove()
    {
        if (!scan_received_)
        {
            ROS_WARN_THROTTLE(1.0, "No scan data received yet");
            return;
        }
        
        double min_range, max_range, min_angle, max_angle;
        findMinMaxRanges(min_range, max_range, min_angle, max_angle);
        
        // Create and publish visualization markers
        marker_pub_.publish(createScanRangeMarker());
        marker_pub_.publish(createPointMarker(1, min_range, min_angle, 1.0, 0.0, 0.0, "closest_point"));
        marker_pub_.publish(createPointMarker(2, max_range, max_angle, 0.0, 1.0, 0.0, "furthest_point"));
        marker_pub_.publish(createDirectionArrow(max_angle));
        
        // Decide on movement
        geometry_msgs::Twist cmd;
        
        // If too close to an obstacle, prioritize rotation away
        if (min_range < min_safe_distance_)
        {
            ROS_WARN_THROTTLE(1.0, "Obstacle detected at %.2fm! Rotating away.", min_range);
            
            // Rotate away from the closest obstacle
            // If obstacle is on the left (positive angle), rotate right (negative)
            cmd.angular.z = -rotation_speed_ * (min_angle > 0 ? 1.0 : -1.0);
            
            // Slow down or stop forward motion
            cmd.linear.x = forward_speed_ * 0.3;
        }
        else
        {
            // Safe to move forward, aim toward the most open direction
            cmd.linear.x = forward_speed_;
            
            // Steer toward the furthest point
            // max_angle is in radians; we'll use proportional control
            cmd.angular.z = rotation_speed_ * 0.5 * max_angle;
        }
        
        // Publish the velocity command
        cmd_pub_.publish(cmd);
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "autonomous_explorer");
    
    AutonomousExplorer explorer;
    
    ros::spin();
    
    return 0;
}
