#!/usr/bin/env python3
import rospy
from duckietown_msgs.msg import Twist2DStamped

def main():
    rospy.init_node('drive_forward', anonymous=False)

    topic = '/lucky/joy_mapper_node/car_cmd'
    rospy.loginfo(f"Publishing car commands to: {topic}")

    pub = rospy.Publisher(topic, Twist2DStamped, queue_size=1)
    rospy.sleep(1.0)

    # Create move forward message
    move_msg = Twist2DStamped()
    move_msg.v = 0.3      # forward velocity
    move_msg.omega = 0.0   # no turning

    # Create stop message
    stop_msg = Twist2DStamped()
    stop_msg.v = 0.0
    stop_msg.omega = 0.0

    # Drive forward for 3 seconds
    rospy.loginfo("Driving forward for 3 seconds...")
    duration = rospy.Duration(7.0)
    rate = rospy.Rate(10)
    start_time = rospy.Time.now()

    while not rospy.is_shutdown() and (rospy.Time.now() - start_time) < duration:
        move_msg.header.stamp = rospy.Time.now()
        pub.publish(move_msg)
        rate.sleep()

    rospy.loginfo("Stopping...")
    stop_msg.header.stamp = rospy.Time.now()
    pub.publish(stop_msg)

if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass
