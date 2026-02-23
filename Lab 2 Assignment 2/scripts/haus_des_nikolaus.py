#!/usr/bin/env python3
import rospy
from duckietown_msgs.msg import Twist2DStamped

def publish_cmd(pub, v, omega, duration, rate):
    msg = Twist2DStamped()
    msg.v = v
    msg.omega = omega
    start = rospy.Time.now()
    while not rospy.is_shutdown() and (rospy.Time.now() - start) < rospy.Duration(duration):
        msg.header.stamp = rospy.Time.now()
        pub.publish(msg)
        rate.sleep()

def stop(pub, pause=2.5):
    msg = Twist2DStamped()
    msg.v = 0.0
    msg.omega = 0.0
    msg.header.stamp = rospy.Time.now()
    pub.publish(msg)
    rospy.sleep(pause)

def main():
    rospy.init_node('haus_des_nikolaus', anonymous=False)
    pub = rospy.Publisher('/lucky/joy_mapper_node/car_cmd', Twist2DStamped, queue_size=1)
    rospy.sleep(1.0)
    rate = rospy.Rate(10)

    speed = 0.3
    turn = 3.0

    side = 2.0
    diag_short = 1.4
    diag_long = 2.8

    t90_roof = 0.95
    t90 = 0.85
    t135 = 1.3

    moves = [
        # 1: bottom left to top left (facing up)
        (speed, 0.0, side),
        # turn right 90
        (0.0, -turn, t90),
        # 2: top left to top right
        (speed, 0.0, side),
        # turn right 90
        (0.0, -turn, t90),
        # 3: top right to bottom right
        (speed, 0.0, side),
        # turn right 90
        (0.0, -turn, t90),
        # 4: bottom right to bottom left
        (speed, 0.0, side),
        # turn right 135
        (0.0, -turn, t135),
        # 5: bottom left to top right (diagonal)
        (speed, 0.0, diag_long),
        # turn left  90
        (0.0, turn, t90_roof),
        # 6: top right to roof peak
        (speed, 0.0, diag_short),
        # turn left  90
        (0.0, turn, t90_roof),
        # 7: roof peak to top left
        (speed, 0.0, diag_short),
        # turn right 45
        (0.0, turn, t135),
        # 8: top left to bottom right (diagonal)
        (speed, 0.0, diag_long),
    ]

    rospy.loginfo("Starting Haus des Nikolaus...")
    for i, (v, omega, dur) in enumerate(moves):
        rospy.loginfo(f"Move {i+1}: v={v}, omega={omega}, dur={dur}")
        publish_cmd(pub, v, omega, dur, rate)
        stop(pub, pause=1.5)

    rospy.loginfo("Haus des Nikolaus complete!")

if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass
