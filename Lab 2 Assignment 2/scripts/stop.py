#!/usr/bin/env python3
import rospy
from duckietown_msgs.msg import Twist2DStamped

rospy.init_node('stop_robot')
pub = rospy.Publisher('/lucky/joy_mapper_node/car_cmd', Twist2DStamped, queue_size=1)
rospy.sleep(0.5)
msg = Twist2DStamped()
msg.v = 0.0
msg.omega = 0.0
msg.header.stamp = rospy.Time.now()
pub.publish(msg)
print("Robot stopped.")
