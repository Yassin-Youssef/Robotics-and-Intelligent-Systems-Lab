#include <ros/ros.h>
#include <geometry_msgs/Twist.h>

int main(int argc, char** argv) {
  ros::init(argc, argv, "circle");
  ros::NodeHandle nh;

  ros::Publisher pub =
      nh.advertise<geometry_msgs::Twist>("/turtle1/cmd_vel", 1);  // lab topic

  ros::Rate loop(10);  // 10 Hz

  while (ros::ok()) {
    geometry_msgs::Twist cmd;
    cmd.linear.x  = 1.0;  // forward speed
    cmd.angular.z = 1.0;  // turn rate → circle
    pub.publish(cmd);
    ros::spinOnce();
    loop.sleep();
  }
  return 0;
}

