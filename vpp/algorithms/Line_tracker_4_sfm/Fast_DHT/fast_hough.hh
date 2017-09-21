#ifndef HOUGH_IMAGE_HH
#define HOUGH_IMAGE_HH



#include <ctime>
#include <cstdlib>

#include <opencv2/highgui.hpp>
#include <vpp/vpp.hh>
#include <Eigen/Core>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/utils/opencv_utils.hh>
#include "boost/intrusive/list.hpp"
#include "dense_one_to_one_hough.hh"
#include "vpp/algorithms/Line_tracker_4_sfm/symbols.hh"
#include "vpp/algorithms/Line_tracker_4_sfm/miscellanous/define.hh"



using namespace vpp;
using namespace std;
using namespace Eigen;
using namespace iod;
using namespace cv;

namespace vpp{

float getVectorVal(std::vector<float> t_array, int vert,int hori,int i ,int j);
void Hough_Accumulator(image2d<uchar> img, int mode , int T_theta, Mat &bv, float acc_threshold);
cv::Mat Hough_Accumulator_Video_Map_and_Clusters(image2d<vuchar1> img, int mode , int T_theta,
                                                 std::vector<float>& t_accumulator, std::list<vint2>& interestedPoints,
                                                 float rhomax);
cv::Mat Hough_Accumulator_Video_Clusters(image2d<vuchar1> img, int mode , int T_theta,
                                         std::vector<float>& t_accumulator,std::list<vint2>& interestedPoints,
                                         float rhomax);
int getThetaMax(Theta_max discr);
cv::Mat accumulatorToFrame(std::vector<float> t_accumulator, float max, int rhomax, int T_theta);
cv::Mat accumulatorToFrame(std::list<vint2> interestedPoints, int rhomax, int T_theta);
void hough_image(int T_theta, float acc_threshold);

/*
template <typename... OPTS>
void Capture_Image(int mode, Theta_max discr , Sclare_rho scale
                   ,Type_video_hough type_video
                   , Type_output type_sortie,
                   Type_Lines type_line,Frequence freq,Mode mode_type,
                   OPTS... options);*/

}

#include "fast_hough.hpp"

#endif // HOUGH_IMAGE_HH
