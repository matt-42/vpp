

A fast embedded implementation of a line detection algorithm using a dense one-to-one Hough transform and a tracking through the Hough space.

## Getting Started

This project contains a fast Dense one-to-one Hough transform based on Video++ library. There is also a line tracking method, to temporally associate every line to its successive positions along the video. Our method then performs mode (cluster) tracking in the Hough space, using prediction and matching of the clusters based on both their position and appearance. The implementation takes advantage of the high performance video processing library Video++, that allows to parallelise simply and efficiently many video primitives.  
We have implement a method to detect vanishing points and the RPLN method (Robuts Pose Estimation from Lines Correspondences) http://www.mip.informatik.uni-kiel.de/tiki-index.php?page=Lilian+Zhang

### Prerequisites

The prerequisites are the same as for Video++. But you must include unsupported eigen modules.


### Installing

The program is furnished as headers so it is enough to install Video++. There a demo of the tracking here :

## Example

You can launch the program by calling :

```c++
Capture_Image(feature_matching_video,Theta_max::SMALL, Sclare_rho::HALF,
                  Type_video_hough::ALL_POINTS,
                  Type_output::ORIGINAL_VIDEO,
                  Type_Lines::ONLY_POLAR,
                  Frequence::ALL_FRAME,
                  With_Kalman_Filter::NO,
                  With_Transparency::YES,
                  With_Entries::NO,
                  _rayon_exclusion_theta = 15,
                  _rayon_exclusion_rho = 12,
                  _slot_hough = 1,
                  _link_of_video_image = "videos/work1.avi",
                  _acc_threshold = 100,
                  _m_first_lines = 5,
                  _max_trajectory_length = 5,
                  _nombre_max_frame_without_update =5);
```



