#include <iostream>
#include <time.h>
#include "hough_extruder_example.hh"

#include <chrono>


using namespace  std;
using namespace Eigen;
using namespace cv;
using namespace vpp;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    // Start the computation of the time used by the function to compute
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    /**
     * @brief fast_dht_matching
     * This function allows to perform fast dense one-to-one Hough Transform
     * The fisrt parameter represents the type media you want to use. you can use either an image or a video
     * The second parameter represents the value of theta you want to use. This parameter defines the height of the array accumulator
     * The third parameter represents the scaling which will be used for rho. This parameter can be used to reduced the size of the accumulator
     * The parameter Type_video_hough represents the way the user wants to use after the tracking
     * The parameter Type_Lines defines the way we describe lines. A line can be described by polar coordinates , the list of all points forming this lines , by the extremities of this lines
     * The parameter With_Kalman_Filter represents the facts if the user wants to use kalman filter to perform prediction
     * The parameter With_Transparency represents the way the user wants to print the trajectories with transpenrcy
     * The parameter With_Entries represents the fact if the user wants the tracking to be performed with entries
     */

    fast_dht_matching(dense_ht,Theta_max::SMALL, Sclare_rho::SAME,
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
                  _link_of_video_image = "m.png",
                  _acc_threshold = 100,
                  _m_first_lines = 5,
                  _max_trajectory_length = 5,
                  _nombre_max_frame_without_update =5);/**/
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    //Show the time used to compute
    cout << "la duree " << duration << endl ;
    return 0;
}
