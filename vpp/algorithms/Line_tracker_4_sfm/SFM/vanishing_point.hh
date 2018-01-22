#pragma once
#include <ctime>
#include <cstdlib>


#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <vpp/vpp.hh>
#include <Eigen/Core>
#include <list>


using namespace vpp;
using namespace std;
using namespace Eigen;
using namespace iod;
using namespace cv;

namespace vpp
{

void get_vanishing_points(int N, std::list<vint2> dominant_lines, int T_theta, int rhomax)
{
    int discr_phi = 1400;
    int discr_the = 1400;
    float scale = rhomax/2.0;
    VectorXf vanish_accumulator(discr_phi*discr_the);
    vanish_accumulator.setZero();
    std::vector<vfloat3> acc_;
    for(auto &dl1 : dominant_lines)
    {
        for(auto &dl2 : dominant_lines)
        {
            if((dl1-dl2).norm()!=0)
            {
                float rho1,rho2,theta1,theta2;
                theta1 = ((2*M_PI*(dl1[1]))/ (T_theta-1)) - M_PI;
                rho1 = dl1[0];
                theta2 = ((2*M_PI*(dl2[1]))/ (T_theta-1)) - M_PI;
                rho2 = dl2[0];
                float x = (rho2/sin(theta2) - rho1/sin(theta1)) / ( 1/tan(theta2) - 1/tan(theta1));
                float y = (rho2 - x*cos(theta2) ) / sin(theta2) ;
                float x_2 = x/2;
                float y_2 = y/2;
                float diag_2 = sqrt(x_2*x_2 + y_2*y_2);
                float phi = acos(y_2/diag_2);
                float big_diag_2 = sqrt(rhomax + diag_2*diag_2);
                float theta = acos(diag_2/big_diag_2);
                int ind_theta = discr_the * theta / (M_PI/2);
                int ind_phi = discr_phi * phi / (2 * M_PI);
                vanish_accumulator[ind_theta*discr_phi + ind_phi]++;
                //cout << "valeurs : x = " << x_2 << " y = " << y_2 << " diag2 " << diag_2 << " bid " << big_diag_2 << endl;
                float ph = 2*M_PI*ind_phi / discr_phi;
                float th = (M_PI/2)*ind_theta / discr_the;
                cout << "valeurs " << ph << "  " << th << endl;
                cout << "valeurs " << phi << "  " << theta << endl << endl << endl;
            }
        }
    }
    std::list<vint3> max_accu;
    for(int t = 0 ; t < discr_the ; t++)
    {
        for(int p = 0 ; p < discr_phi ; p++)
        {
            int ind = t*discr_phi + p ;
            if(vanish_accumulator[ind]>0)
            {
                cout << "valeur accumulée " << vanish_accumulator[ind] << endl;
                max_accu.push_back(vint3(t,p,vanish_accumulator[ind]));
            }

        }
    }
    max_accu.sort( [&](vint3& a, vint3& b){return a[2] > b[2];});
    int i =0;
    for(auto &val : max_accu)
    {
        int t = val[0];
        int p = val[1];
        float phi = 2*M_PI*p / discr_phi;
        float theta = 2*M_PI * t / discr_the;
        cout << " les angles  phi :" << phi << " theta " << theta << endl;
        if(i==2)
            break;
        i++;
    }
}


void get_vanishing_points1(int N, std::vector<vfloat3> dominant_lines, int T_theta, int rhomax, int nrows,int ncols)
{
    //int discr_phi = 1400;
    //int discr_the = 1400;
    //float scale = rhomax/2.0;
    //VectorXf vanish_accumulator(discr_phi*discr_the);
    std::vector<vfloat4> coord(3,vfloat4(0,0,0,-1));
    for(int i = 0 ; i < 3 ; i++)
    {
        (coord[i])[3] = i;
    }
    //vanish_accumulator.setZero();
    float mx = ncols/2;
    float my = nrows/2;
    std::vector<vfloat3> acc_;
    std::vector<vfloat3> cluster1;
    std::vector<vfloat3> cluster2;
    std::vector<vfloat3> cluster3;
    std::vector<vfloat3> outliers;
    for(int i = 0 ; i < dominant_lines.size(); i++)
    {
        cout <<" i = " << i << endl;
        for(int j = i+1 ; j < dominant_lines.size(); j++)
        {
            if(j!=i && j < dominant_lines.size())
            {
                cout << " j = " << j << endl;
                float rho1,rho2,theta1,theta2;
                theta1 = ((2*M_PI*(dominant_lines[i])[1])/ (T_theta-1)) - M_PI;
                rho1 = (dominant_lines[i])[0];
                theta2 = ((2*M_PI*(dominant_lines[j])[1])/ (T_theta-1)) - M_PI;
                rho2 = (dominant_lines[j])[0];
                float x = (rho2/sin(theta2) - rho1/sin(theta1)) / ( 1/tan(theta2) - 1/tan(theta1));
                float y = (rho2 - x*cos(theta2) ) / sin(theta2);
                if(fabs(theta1-theta2)<0.1)
                {
                    vfloat3 point= vfloat3(x,y,2);
                    for(int k = 0 ; k < dominant_lines.size() ; k++)
                    {
                        if(k!=i && k!=j)
                        {
                            float rho3,theta3;
                            rho3 = (dominant_lines[k])[0];
                            theta3 = ((2*M_PI*(dominant_lines[k])[1])/ (T_theta-1)) - M_PI;
                            float res = cos(theta3)*x + sin(theta3)*y - rho3;
                            if(fabs(res) < 10)
                            {
                                point[2]++;
                            }
                        }
                    }
                    acc_.push_back(point);
                    cout << " x " << point[0] << " y " << point[1] << " val " << point[2] << endl;
                }
            }
        }
    }
    std::sort(acc_.begin(), acc_.end(),[&](vfloat3& a, vfloat3& b){return a[2] > b[2];});
    //std::vector<vint2> vanishing_point(3);
    double inf = std::numeric_limits<float>::infinity();
    for(int i = 0 ; i < acc_.size() ; i++)
    {
        if((acc_[i])[2]>=2)
        {
            cout << " x " << (acc_[i])[0] << " y " << (acc_[i])[1] << " val " << (acc_[i])[2] << endl;
            if(i==0)
            {
                cluster1.push_back(acc_[i]);
                continue;
            }
            if( fabs((acc_[i])[0]) == inf && fabs((acc_[i])[1]) == inf)
            {
                if( cluster1.size()>0 &&  (acc_[i])[0]==(cluster1[0])[0]   && (acc_[i])[1]==(cluster1[0])[1]   )
                {
                    cluster1.push_back(acc_[i]);
                }
                else if( cluster2.size()>0 &&  (acc_[i])[0]==(cluster2[0])[0]   && (acc_[i])[1]==(cluster2[0])[1]   )
                {
                    cluster2.push_back(acc_[i]);
                }
                else if( cluster3.size()>0 &&  (acc_[i])[0]==(cluster3[0])[0]   && (acc_[i])[1]==(cluster3[0])[1]   )
                {
                    cluster3.push_back(acc_[i]);
                }
            }
            else
            {
                if( cluster1.size()>0 && ( (acc_[i])[0]*(cluster1[0])[0] > 0  && (acc_[i])[1]*(cluster1[0])[1] > 0 ) )
                {
                    if(( (acc_[i]).segment(0,1) - (cluster1[0]).segment(0,1) ).norm() < 100  )
                    cluster1.push_back(acc_[i]);
                }
                else if(  cluster2.size()>0 && ( (acc_[i])[0]*(cluster2[0])[0] > 0  && (acc_[i])[1]*(cluster2[0])[1] > 0 ) )
                {
                    if(( (acc_[i]).segment(0,1) - (cluster2[0]).segment(0,1) ).norm() < 100 )
                    cluster2.push_back(acc_[i]);
                }
                else if(  cluster3.size()>0 && ( (acc_[i])[0]*(cluster3[0])[0] > 0  && (acc_[i])[1]*(cluster3[0])[1] > 0 )  )
                {
                          if(( (acc_[i]).segment(0,1) - (cluster3[0]).segment(0,1) ).norm() < 100)
                    cluster3.push_back(acc_[i]);
                }
                else
                {
                    if(cluster1.size()==0)
                    {
                        cluster1.push_back(acc_[i]);
                    }
                    else if(cluster2.size()==0)
                    {
                        cluster2.push_back(acc_[i]);
                    }
                    else if(cluster3.size()==0)
                    {
                        cluster3.push_back(acc_[i]);
                    }
                    else
                    {
                        outliers.push_back(acc_[i]);
                    }
                }
            }
        }
    }
    vfloat3 C1;
    vfloat3 C2;
    vfloat3 C3;
    if(cluster1.size() == 0 || cluster2.size() == 0 || cluster3.size() == 0 )
    {
        return;
    }

    if(cluster1.size()>0 &&  fabs((cluster1[0])[0]) == inf && fabs((cluster1[0])[1]) == inf )
    {
        C1 = cluster1[0];
    }
    else if ( cluster1.size()>0 )
    {
        float cf1 = 0;
        cout << "cluster 1" << endl;
        for(int i = 0 ; i < cluster1.size() ; i++)
        {
            cout << " x " << (cluster1[i])[0] << " y " << (cluster1[i])[1] << " val " << (cluster1[i])[2] << endl;
            C1[0] += (cluster1[i])[0] * (cluster1[i])[2];
            C1[1] += (cluster1[i])[1] * (cluster1[i])[2];
            cf1 += (cluster1[i])[2];
        }
        C1[0] /= cf1;
        C1[1] /= cf1;
        C1[2] = -1;
    }
    cout << endl << endl << endl;

    if(cluster2.size()>0 &&  fabs((cluster2[0])[0]) == inf && fabs((cluster2[0])[1]) == inf )
    {
        C2 = cluster2[0];
    }
    else if ( cluster2.size()>0 )
    {
        float cf2 = 0;
        cout << "cluster 2" << endl;
        for(int i = 0 ; i < cluster2.size() ; i++)
        {
            cout << " x " << (cluster2[i])[0] << " y " << (cluster2[i])[1] << " val " << (cluster2[i])[2] << endl;
            C2[0] += (cluster2[i])[0] * (cluster2[i])[2];
            C2[1] += (cluster2[i])[1] * (cluster2[i])[2];
            cf2 += (cluster2[i])[2];
        }
        C2[0] /= cf2;
        C2[1] /= cf2;
        C2[2] = -1;
    }

    cout << endl << endl << endl;

    if(cluster3.size()>0 &&  fabs((cluster3[0])[0]) == inf && fabs((cluster3[0])[1]) == inf )
    {
        C3 = cluster3[0];
    }
    else if ( cluster3.size()>0 )
    {
        float cf3 = 0;
        cout << "cluster 3" << endl;
        for(int i = 0 ; i < cluster3.size() ; i++)
        {
            cout << " x " << (cluster3[i])[0] << " y " << (cluster3[i])[1] << " val " << (cluster3[i])[2] << endl;
            C3[0] += (cluster3[i])[0] * (cluster3[i])[2];
            C3[1] += (cluster3[i])[1] * (cluster3[i])[2];
            cf3 += (cluster3[i])[2];
        }
        C3[0] /= cf3;
        C3[1] /= cf3;
        C3[2] = -1;
    }

    cout << endl << endl << endl;

    cout << "cluster 1 " << C1 << endl;
    cout << "cluster 2 " << C2 << endl;
    cout << "cluster 3 " << C3 << endl;/**/

}

void get_vanishing_points2(int N, std::vector<vfloat3> dominant_lines, int T_theta, int rhomax)
{
    int discr_phi = 1400;
    int discr_the = 1400;
    float scale = rhomax/2.0;
    VectorXf vanish_accumulator(discr_phi*discr_the);
    vanish_accumulator.setZero();
    std::vector<vfloat3> acc_;
    for(int i = 0 ; i < dominant_lines.size(); i++)
    {
        cout <<" i = " << i << endl;
        for(int j = i+1 ; j < dominant_lines.size(); j++)
        {
            if(j!=i && j < dominant_lines.size())
            {
                cout << " j = " << j << endl;
                float rho1,rho2,theta1,theta2;
                theta1 = ((2*M_PI*(dominant_lines[i])[1])/ (T_theta-1)) - M_PI;
                rho1 = (dominant_lines[i])[0];
                theta2 = ((2*M_PI*(dominant_lines[j])[1])/ (T_theta-1)) - M_PI;
                rho2 = (dominant_lines[j])[0];
                float x = (rho2/sin(theta2) - rho1/sin(theta1)) / ( 1/tan(theta2) - 1/tan(theta1));
                float y = (rho2 - x*cos(theta2) ) / sin(theta2);
                if(fabs(theta1-theta2)<0.5)
                {
                    vfloat3 point= vfloat3(x,y,(dominant_lines[i])[2] + (dominant_lines[j])[2]);
                    for(int k = 0 ; k < dominant_lines.size() ; k++)
                    {
                        if(k!=i && k!=j)
                        {
                            float rho3,theta3;
                            rho3 = (dominant_lines[k])[0];
                            theta3 = ((2*M_PI*(dominant_lines[k])[1])/ (T_theta-1)) - M_PI;
                            float res = cos(theta3)*x + sin(theta3)*y - rho3;
                            if(fabs(res) < 50)
                            {
                                point[2] += (dominant_lines[k])[2];
                            }
                        }
                    }
                    acc_.push_back(point);
                    cout << " x " << point[0] << " y " << point[1] << " val " << point[2] << endl;
                }
            }
        }
    }
    std::sort(acc_.begin(), acc_.end(),[&](vfloat3& a, vfloat3& b){return a[2] > b[2];});
    for(int i = 0 ; i < acc_.size() ; i++)
    {
        if((acc_[i])[2]>2)
        {
            cout << " x " << (acc_[i])[0] << " y " << (acc_[i])[1] << " val " << (acc_[i])[2] << endl;
        }
    }
}


inline
float compute_a(float thetaj)
{
    return 2*M_PI*(1-cos(thetaj));
}


inline
float compute_theta(float nz,float nx,float ny, float phi)
{
    return asin(nz/(sqrt(nz*nz+ pow(nx*cos(phi)+ny*sin(phi),2))));
}

inline
float compute_psi(float nz,float nx,float ny, float phi)
{
    float alpha = nx*cos(phi) + ny*sin(phi);
    return M_PI/2 * alpha /(sqrt(nz*nz + alpha*alpha));
}

}

