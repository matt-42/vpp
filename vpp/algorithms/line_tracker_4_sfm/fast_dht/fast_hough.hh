#pragma once


#include <ctime>
#include <cstdlib>

#include <opencv2/highgui.hpp>
#include <vpp/vpp.hh>
#include <Eigen/Core>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/utils/opencv_utils.hh>
#include "dense_one_to_one_hough.hh"
#include "vpp/algorithms/line_tracker_4_sfm.hh"
#include "vpp/algorithms/line_tracker_4_sfm/miscellanous/define.hh"



using namespace vpp;
using namespace std;
using namespace Eigen;
using namespace iod;
using namespace cv;

namespace vpp{

float get_vector_val(std::vector<float> t_array, int vert,int hori,int i ,int j);
void hough_accumulator(image2d<uchar> img, int T_theta, Mat &bv, float acc_threshold);
cv::Mat hough_accumulator_video_map_and_clusters(image2d<vuchar1> img, int mode , int T_theta,
                                                 std::vector<float>& t_accumulator, std::list<vint2>& interestedPoints,
                                                 float rhomax);
cv::Mat hough_accumulator_video_clusters(image2d<vuchar1> img, int mode , int T_theta,
                                         std::vector<float>& t_accumulator,std::list<vint2>& interestedPoints,
                                         float rhomax);
int get_theta_max(Theta_max discr);
cv::Mat accumulator_to_frame(std::vector<float> t_accumulator, float max, int rhomax, int T_theta);
cv::Mat accumulator_to_frame(std::list<vint2> interestedPoints, int rhomax, int T_theta);
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

