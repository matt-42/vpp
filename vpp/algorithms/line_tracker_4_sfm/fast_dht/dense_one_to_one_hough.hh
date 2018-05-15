#pragma once


#include "openacc.h"
#include "vpp/algorithms/line_tracker_4_sfm/miscellanous.hh"
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <vpp/vpp.hh>
#include <Eigen/Core>
#include <vpp/utils/opencv_bridge.hh>
#include <iod/timer.hh>

using namespace vpp;
using namespace std;
using namespace Eigen;
using namespace iod;
using namespace cv;

namespace vpp{

#define Two_PI 2*M_PI


std::list<vint2> Hough_Lines_Parallel(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu
                                      ,  image2d<uchar> &out
                                      , std::vector<std::list<vint2> > &points50);
std::list<vint2> Hough_Lines_Parallel_Kmeans(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max,int rhomax, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img );
std::list<vint2> Hough_Lines_Parallel(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img, std::vector<vint4>& extremites);
std::list<vint2> Hough_Lines_Parallel(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img);
std::list<vint2> Hough_Lines_Parallel_Update_Threshold(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max,int rhomax, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img , float& grad_threshold , int N);
std::list<vint2> Hough_Lines_Parallel_Sparse(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, int rhomax, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img , float grad_threshold , int N);
std::list<vint2> Hough_Lines_Parallel_Sparse1(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img);
std::list<vint2> Hough_Lines_Parallel(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, int rhomax, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img , float acc_threshold);

std::list<vint2> Hough_Lines_Parallel_one(Mat bv,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu
                                      ,  Mat &grad_img);

std::list<vint2> Hough_Lines_Parallel_one(image2d<uchar> img, Mat bv,
                           std::vector<float>& t_accumulator,
                           int Theta_max, float& max_of_the_accu
                           ,  Mat &grad_img , std::vector<LineIterator> list_interest);

void Hough_Lines_Sequentiel(image2d<vuchar1> img,int sigma,int nbsigma,std::vector<float>& t_accumulator, int nrows, int ncols);


std::list<vint2> Hough_Lines_Parallel_Gradient(image2d<uchar> img,
                                          std::vector<float>& t_accumulator,
                                          int Theta_max, float& max_of_the_accu,
                                               image2d<uchar> &grad_img, std::vector<vint4> &points50);
std::list<vint2> Hough_Lines_Parallel_Basic(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu);
std::priority_queue<vint2> Hough_Lines_Parallel_V3(image2d<vuchar1> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu, int threshold,
                                         image2d<vuchar3> &cluster_colors , int nb_old);
void adap_thresold(std::list<vfloat3> &list_temp , float &threshold_hough , int &calls ,
                   int &nb_calls_limits_reached , int rhomax, int T_theta, std::vector<float> t_accumulator);
void reduce_number_of_max_local(std::list<vfloat3> &list_temp , float threshold_hough , int rhomax, int T_theta , std::vector<float> t_accumulator);

void Hough_Lines_Parallel_Map(image2d<vuchar1> img);

}
#include "dense_one_to_one_hough.hpp"


