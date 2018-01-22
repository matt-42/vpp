#pragma once
#include <vector>
#include <opencv2/highgui.hpp>
#include <vpp/vpp.hh>
#include <Eigen/Core>


using namespace Eigen;

using namespace vpp;




double execution_time = 0;
double execution_t= 0;




enum class Theta_max : int32_t { XXSMALL = 67 , XSMALL = 127 , SMALL = 255, MEDIUM = 500, LARGE = 1000 , XLARGE = 1500};
enum class Sclare_rho : int32_t { ONE_QUART = 3, HALF = 2 , THREE_QUART = 1 , SAME = 0};
enum class Type_video_hough : int8_t { ONLY_CLUSTERS = 1 , ALL_POINTS = 2 };
enum class Type_capture : int16_t { webcam = 0 , photo = 1 , video = 3 } ;
enum class Type_tracking : int16_t { no_tracking = 0 , lucas_kanade = 1 , kalman_track = 2 , video_extruder_vid = 3};
enum class Type_output : int16_t { VIDEO_HOUGH = 0 , ORIGINAL_VIDEO = 1 , GRADIENT_VIDEO = 2,ORIGINAL_AND_HOUGH_VIDEO = 3   };
enum class Type_Lines : int8_t { EXTREMITE = 0 , ALL_POINTS = 1 , ONLY_POLAR = 2 };
enum class Frequence : int8_t { ALL_FRAME = 0 , NOT_ALL = 1 };
enum class Mode : int8_t { ONLY_VPP = 0 , OPTIMIZED = 1 };
enum class With_Kalman_Filter : int8_t { YES = 1 , NO = 0 };
enum class With_Transparency : int8_t { YES = 1 , NO = 0 };
enum class With_Entries : int8_t { YES = 1 , NO = 0 };

float  motion_threshold = 100;
#define dense_ht 2
#define feature_matching 7
Matrix<float,3,3> GySobel3x3;
Matrix<float,3,3> GxSobel3x3;
Matrix<float,5,5> GySobel5x5;
Matrix<float,5,5> GxSobel5x5;
Matrix<float,7,7> GySobel7x7;
Matrix<float,7,7> GxSobel7x7;
Matrix<float,9,9> GySobel9x9;
Matrix<float,9,9> GxSobel9x9;


