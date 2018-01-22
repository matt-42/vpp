#pragma once

#include <ctime>
#include <cstdlib>

#include <iod/parse_command_line.hh>
#include <iod/timer.hh>

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <vpp/vpp.hh>
#include <Eigen/Core>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/video_extruder.hh>
#include <vpp/utils/opencv_utils.hh>
#include <vpp/draw/draw_trajectories.hh>
#include <vpp/algorithms/Line_tracker_4_sfm/Hough_Extruder/draw_trajectories_hough.hh>
#include "vpp/algorithms/Line_tracker_4_sfm/symbols.hh"
#include "vpp/algorithms/Line_tracker_4_sfm/Hough_Extruder/paint.hh"
#include "vpp/algorithms/Line_tracker_4_sfm/Hough_Extruder/feature_matching_hough.hh"

