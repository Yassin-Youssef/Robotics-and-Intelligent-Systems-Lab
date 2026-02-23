#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <turtlesim/Pose.h>
#include <turtlesim/TeleportAbsolute.h>
#include <turtlesim/SetPen.h>
#include <std_srvs/Empty.h>
#include <cmath>

static turtlesim::Pose POSE; static bool HAVE_POSE=false;
double clamp(double v, double lo, double hi){ return std::max(lo,std::min(hi,v)); }
double normAng(double a){ while(a>M_PI) a-=2*M_PI; while(a<-M_PI)a+=2*M_PI; return a; }
void poseCb(const turtlesim::Pose& p){ POSE=p; HAVE_POSE=true; }

void stop(ros::Publisher& pub){
  geometry_msgs::Twist z; pub.publish(z);
}

void rotateTo(ros::Publisher& pub, double theta_target){
  ros::Rate r(100);
  const double K=6.0, WMAX=3.5, ATOL=0.01; // ~0.6°
  while(ros::ok()){
    ros::spinOnce(); if(!HAVE_POSE){ r.sleep(); continue; }
    double e = normAng(theta_target - POSE.theta);
    if (std::fabs(e) < ATOL) break;
    geometry_msgs::Twist cmd; cmd.angular.z = clamp(K*e, -WMAX, WMAX);
    pub.publish(cmd);
    r.sleep();
  }
  stop(pub);
}

void driveStraightTo(ros::Publisher& pub, double x, double y){
  ros::Rate r(100);
  const double VMAX=2.0, Kp=2.0, Kh=8.0, DTOL=0.01, HTOL=0.02; // tight
  // Fix heading first
  double desired = std::atan2(y-POSE.y, x-POSE.x);
  rotateTo(pub, desired);

  // Drive straight, keep heading tightly
  while(ros::ok()){
    ros::spinOnce(); if(!HAVE_POSE){ r.sleep(); continue; }
    double dx=x-POSE.x, dy=y-POSE.y;
    double d = std::sqrt(dx*dx+dy*dy);
    if (d < DTOL) break;

    double heading = std::atan2(dy,dx);
    double eh = normAng(heading - POSE.theta);

    geometry_msgs::Twist cmd;
    cmd.linear.x  = clamp(Kp * d, 0.0, VMAX);
    // very strong heading hold; if off too much, stop forward motion to keep line crisp
    if (std::fabs(eh) > HTOL) cmd.linear.x = 0.0;
    cmd.angular.z = clamp(Kh * eh, -3.5, 3.5);

    pub.publish(cmd);
    r.sleep();
  }
  stop(pub);
}

int main(int argc, char** argv){
  ros::init(argc, argv, "house_sharp");
  ros::NodeHandle nh;

  ros::Subscriber sub = nh.subscribe("/turtle1/pose", 1, poseCb);
  ros::Publisher  pub = nh.advertise<geometry_msgs::Twist>("/turtle1/cmd_vel", 1);

  ros::ServiceClient clear  = nh.serviceClient<std_srvs::Empty>("/clear");
  ros::ServiceClient tele   = nh.serviceClient<turtlesim::TeleportAbsolute>("/turtle1/teleport_absolute");
  ros::ServiceClient setpen = nh.serviceClient<turtlesim::SetPen>("/turtle1/set_pen");

  ros::service::waitForService("/clear");
  ros::service::waitForService("/turtle1/teleport_absolute");
  ros::service::waitForService("/turtle1/set_pen");
  while(ros::ok() && !HAVE_POSE){ ros::spinOnce(); ros::Duration(0.01).sleep(); }

  // Pen up, move to start, clear, pen down (crisp canvas)
  turtlesim::SetPen pen; pen.request.r=255; pen.request.g=255; pen.request.b=255; pen.request.width=2; pen.request.off=1; setpen.call(pen);
  turtlesim::TeleportAbsolute tp; tp.request.x=3.0; tp.request.y=3.0; tp.request.theta=0.0; tele.call(tp);
  std_srvs::Empty e; clear.call(e);
  pen.request.off=0; setpen.call(pen);

  // Coordinates that fit nicely
  struct P{double x,y;};
  P A{3.0,3.0}, B{6.0,3.0}, C{6.0,6.0}, D{3.0,6.0}, E{4.5,7.5};

  // --- Draw a clean Haus des Nikolaus (not single-stroke; crisp straight edges) ---
  // Square base
  driveStraightTo(pub, B.x, B.y); // A->B
  driveStraightTo(pub, C.x, C.y); // B->C
  driveStraightTo(pub, D.x, D.y); // C->D
  driveStraightTo(pub, A.x, A.y); // D->A

  // Diagonal A->C
  driveStraightTo(pub, C.x, C.y); // A->C

  // Roof edges C->E->D
  driveStraightTo(pub, E.x, E.y); // C->E
  driveStraightTo(pub, D.x, D.y); // E->D

  // Second diagonal to complete the X
  driveStraightTo(pub, B.x, B.y); // D->B

  // Done
  stop(pub);
  ros::spin();
  return 0;
}

