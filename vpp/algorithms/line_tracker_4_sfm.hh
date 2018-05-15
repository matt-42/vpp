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
#include "line_tracker_4_sfm/hough_extruder.hh"
#include "line_tracker_4_sfm/fast_dht.hh"
#include "line_tracker_4_sfm/sfm.hh"
#include "line_tracker_4_sfm/miscellanous.hh"

