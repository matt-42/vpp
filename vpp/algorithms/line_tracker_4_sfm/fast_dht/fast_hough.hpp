

#include "fast_hough.hh"
#include "vpp/algorithms/line_tracker_4_sfm/hough_extruder.hh"
#include "vpp/algorithms/line_tracker_4_sfm/miscellanous.hh"

namespace vpp{

int SobelX[3][3] =
{
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
};

int SobelY[3][3] =
{
    1, 2, 1,
    0, 0, 0,
    -1,-2,-1
};




template <typename type_>
inline
float function_diff(int k1,int k2)
{
    return fabs((k1-k2)/(k1+k2));
}


class compare_1 {
public:
    bool operator()(const int x,const int y) {
        return x>y;
    }
};



void hough_accumulator(image2d<uchar> img, int T_theta,Mat &bv,float acc_threshold)
{
    int ncols,nrows;
    ncols = img.ncols();
    nrows = img.nrows();
    int rhomax=int(sqrt(pow(ncols,2)+pow(nrows,2)));
    int thetamax = T_theta;
    float max_of_accu = 0;
    image2d<uchar> out(img.domain());
    std::vector<float> t_accumulator(rhomax*thetamax,0);
        Hough_Lines_Parallel(img,t_accumulator,thetamax,rhomax,max_of_accu,out,acc_threshold);
}

Mat hough_accumulator_video_map_and_clusters(image2d<vuchar1> img, int mode, int T_theta,
                                             std::vector<float>& t_accumulator, std::list<vint2> &interestedPoints, float rhomax)
{
    float max_of_accu = 0;
        cout << "Parallel" << endl;
        //interestedPoints = Hough_Lines_Parallel(img,t_accumulator,T_theta,max_of_accu,5);
        return accumulator_to_frame(t_accumulator,
                                  max_of_accu
                                  ,rhomax,T_theta);
}

cv::Mat hough_accumulator_video_clusters(image2d<vuchar1> img, int mode , int T_theta,
                                         std::vector<float>& t_accumulator, std::list<vint2>& interestedPoints, float rhomax)
{
    float max_of_accu = 0;
        cout << "Parallel" << endl;
        //interestedPoints = Hough_Lines_Parallel(img,t_accumulator,T_theta,max_of_accu,5);
        return accumulator_to_frame(interestedPoints,rhomax,T_theta);
}


int get_theta_max(Theta_max discr)
{
    if(Theta_max::XLARGE == discr)
        return 1500;
    else if(Theta_max::LARGE == discr)
        return 1000;
    else if(Theta_max::MEDIUM == discr)
        return 500;
    else if(Theta_max::SMALL == discr)
        return 255;
    else
        return 0;
}

int get_WKF(With_Kalman_Filter wkf)
{
    if(With_Kalman_Filter::YES == wkf)
    {
        return 1;
    }
    else{
        return 0;
    }
}

float get_scale_rho(Sclare_rho discr)
{
    if(Sclare_rho::SAME == discr)
        return 1;
    else if(Sclare_rho::THREE_QUART == discr)
        return 0.75;
    else if(Sclare_rho::HALF == discr)
        return 0.5;
    else if(Sclare_rho::ONE_QUART == discr)
        return 0.25;
    else
        return 0;
}

cv::Mat accumulator_to_frame(std::vector<float> t_accumulator, float big_max, int rhomax, int T_theta)
{
    Mat T(int(rhomax),int(T_theta),CV_8UC1);
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
#pragma omp parallel for
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float res =  ( t_accumulator[rho*T_theta + theta] * 255.0 ) / big_max;
            uchar encoded = ( pow( (res / 255) , (1 / 2.2))) * 255;
            T.at<uchar>(rho,theta) = encoded;
        }
    }
    return T;
}

cv::Mat accumulator_to_frame(std::list<vint2> interestedPoints, int rhomax, int T_theta)
{
    Mat T = Mat(int(rhomax),int(T_theta),CV_8UC1,cvScalar(0));
    int r = 0;
    for(auto& ip : interestedPoints)
    {
        //int radius = round(5-0.1*r);
        circle(T,cv::Point(ip[1],ip[0]),1,Scalar(255),CV_FILLED,8,0);
        r++;
        //break;
    }
    return T;
}

void hough_image(int T_theta,float acc_threshold,const char * lien_image = "m.png")
{
    typedef image2d<uchar> Image;
    Mat bv = cv::imread(lien_image,0);
    Image img = (from_opencv<uchar>(bv));
    //timer t;
    //t.start();
    //for(int i=0 ; i < 1000 ; i++)
    {
        hough_accumulator(img,T_theta,bv,acc_threshold);
    }
    //t.end();
    //cout << " temps d'execution " << t.us()/1000.0 << endl;
}




}



