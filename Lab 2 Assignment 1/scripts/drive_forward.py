#!/usr/bin/env python3
import rospy
from duckietown_msgs.msg import WheelsCmdStamped

def main():
    # Initialize the ROS node. anonymous=False so we get a clean name.
    rospy.init_node('drive_forward', anonymous=False)

    # Construct the topic name using the robot's namespace
    topic = f'/luna/wheels_driver_node/wheels_cmd'
    rospy.loginfo(f"Publishing wheel commands to: {topic}")

    pub = rospy.Publisher(topic, WheelsCmdStamped, queue_size=1)
    rospy.sleep(1.0)
    forward_speed = 0.3

    # Create the "move forward" message
    move_msg = WheelsCmdStamped()
    move_msg.vel_left = forward_speed
    move_msg.vel_right = forward_speed

    # Create the "stop" message
    stop_msg = WheelsCmdStamped()
    stop_msg.vel_left = 0.0
    stop_msg.vel_right = 0.0

    # Drive forward for 3 seconds
    rospy.loginfo("Driving forward for 3 seconds...")
    duration = rospy.Duration(3.0)
    rate = rospy.Rate(10)  # Publish at 10 Hz for smooth control
    start_time = rospy.Time.now()

    while not rospy.is_shutdown() and (rospy.Time.now() - start_time) < duration:
        move_msg.header.stamp = rospy.Time.now()
        pub.publish(move_msg)
        rate.sleep()

    rospy.loginfo("Stopping wheels...")
    stop_msg.header.stamp = rospy.Time.now()
    pub.publish(stop_msg)

if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass
