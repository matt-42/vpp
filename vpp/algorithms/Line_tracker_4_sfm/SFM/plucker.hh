#ifndef PLUCKER_HH
#define PLUCKER_HH

#include <vpp/vpp.hh>
#include <list>
#include <iostream>
#include <iterator>
#include <random>

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <Eigen/Core>

using namespace vpp;
using namespace std;
using namespace cv;
using namespace Eigen;

// Inhomogeneous coordinates
class Point3D
{
public: double x,y,z;

    Point3D()
    {x=y=z=0.0;}

    Point3D(double xx, double yy, double zz)
    {x=xx;y=yy;z=zz;}

    friend ostream & operator << (ostream & os, const Point3D p)
    {
        os<<"["<<p.x<<","<<p.y<<","<<p.z<<"]";
        return os;
    }
};

class Plucker{
public: double c[6];


    Plucker(Point3D p1, Point3D p2)
    {
        c[0]=p2.x-p1.x;
        c[1]=p2.y-p1.y;
        c[2]=p2.z-p1.z;
        c[3]=p1.y*p2.z-p1.z*p2.y;
        c[4]=p2.x*p1.z-p1.x*p2.z;
        c[5]=p1.x*p2.y-p2.x*p1.y;
    }




    friend ostream & operator << (ostream & os, const Plucker l)
    {
        os<<"Plucker Line:["<< l.c[0] <<"," << l.c[1] <<","<<l.c[2]<<","<<l.c[3]<<","<<l.c[4]<<","<<l.c[5]<<"]";
        return os;
    }


}; // Homogeneous coordinates

double PluckerDotProduct(Plucker l1, Plucker l2)
{
    double s=0;

    for(int i=0;i<6;i++)
        s+=l1.c[i]*l2.c[(3+i)%6];

    return s;
}


// The same primitive rewritten differently
double PluckerLineIntersect(Plucker l1, Plucker l2)
{
    double s;

    s=l1.c[0]*l2.c[3]+l2.c[0]*l1.c[3] +\
            l1.c[1]*l2.c[4]+l2.c[1]*l1.c[4]+\
            l1.c[2]*l2.c[5]+l2.c[2]*l1.c[5];

    return s;
}


double PluckerPointOnLine(Plucker l, Point3D p)
{
    double rx, ry, rz, rw;

    rx=l.c[3]*p.x+l.c[4]*p.y+l.c[5]*p.z;
    ry=-l.c[2]*p.y+l.c[1]*p.z+l.c[3];
    rz=-l.c[2]*p.x+l.c[0]*p.z-l.c[4];
    rw=-l.c[1]*p.x+l.c[0]*p.y+l.c[5];

    //cout<<rx<<" "<<ry<< " "<<rz<<" "<<rw<<" "<<endl;
    return fabs(rx)+fabs(ry)+fabs(rz)+fabs(rw);
}

#endif // PLUCKER_HH
