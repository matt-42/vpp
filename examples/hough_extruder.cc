#include <iostream>
#include <time.h>
#include "vpp/algorithms/Line_tracker_4_sfm/Hough_Extruder/hough_extruder.hh"

#include <chrono>
//1

using namespace  std;
using namespace Eigen;
using namespace cv;
using namespace vpp;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    Fast_DHT_Matching(feature_matching,Theta_max::SMALL, Sclare_rho::SAME,
                  Type_video_hough::ALL_POINTS,
                  Type_output::ORIGINAL_VIDEO,
                  Type_Lines::ONLY_POLAR,
                  Frequence::ALL_FRAME,
                  With_Kalman_Filter::NO,
                  With_Transparency::YES,
                  With_Entries::YES,
                  _rayon_exclusion_theta = 15,
                  _rayon_exclusion_rho = 12,
                  _slot_hough = 1,
                  _link_of_video_image = "videos/corridor2.mp4",
                  _acc_threshold = 100,
                  _m_first_lines = 5,
                  _max_trajectory_length = 5,
                  _nombre_max_frame_without_update =5);/**/
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>( t2 - t1 ).count();
    cout << "la duree " << duration << endl ;
    return 0;
}
