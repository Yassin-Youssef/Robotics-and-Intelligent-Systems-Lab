#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <tf/transform_listener.h>

int main(int argc, char** argv)
{
    // Initialize ROS and create a node handle
    ros::init(argc, argv, "marker_publisher");
    ros::NodeHandle nh;
    
    // Create a publisher for visualization markers
    // The topic name "visualization_marker" is a standard convention
    ros::Publisher marker_pub = nh.advertise<visualization_msgs::Marker>("visualization_marker", 1);
    
    // Wait a moment for the publisher to establish connections
    ros::Duration(0.5).sleep();
    
    ROS_INFO("Marker publisher node started. Publishing sphere 1m in front of robot.");
    
    // Set up the publishing loop at 10 Hz
    ros::Rate rate(10.0);
    
    while (ros::ok())
    {
        // Create a marker message
        visualization_msgs::Marker marker;
        
        // Set the frame ID - this is crucial!
        // base_footprint is the robot's frame, so the marker moves with the robot
        marker.header.frame_id = "base_footprint";
        marker.header.stamp = ros::Time::now();
        
        // Set a namespace and ID for this marker
        // Namespace groups related markers, ID uniquely identifies this one
        marker.ns = "robot_front_sphere";
        marker.id = 0;
        
        // Set the marker type to SPHERE
        marker.type = visualization_msgs::Marker::SPHERE;
        
        // Set the marker action to ADD (create or update)
        marker.action = visualization_msgs::Marker::ADD;
        
        // Set the position of the sphere
        // In the robot's frame, X is forward, so (1, 0, 0) is 1 meter in front
        marker.pose.position.x = 1.0;
        marker.pose.position.y = 0.0;
        marker.pose.position.z = 0.5;  // Elevated so it's visible
        
        // Set the orientation (spheres don't have orientation, but we must set it)
        marker.pose.orientation.x = 0.0;
        marker.pose.orientation.y = 0.0;
        marker.pose.orientation.z = 0.0;
        marker.pose.orientation.w = 1.0;
        
        // Set the scale of the sphere (0.2m diameter)
        marker.scale.x = 0.2;
        marker.scale.y = 0.2;
        marker.scale.z = 0.2;
        
        // Set the color (bright blue with full opacity)
        marker.color.r = 0.0f;
        marker.color.g = 0.5f;
        marker.color.b = 1.0f;
        marker.color.a = 1.0;
        
        // Marker lifetime (0 means forever)
        marker.lifetime = ros::Duration();
        
        // Publish the marker
        marker_pub.publish(marker);
        
        // Process any callbacks and sleep to maintain rate
        ros::spinOnce();
        rate.sleep();
    }
    
    return 0;
}
