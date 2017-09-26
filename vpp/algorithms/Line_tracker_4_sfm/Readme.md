

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
Capture_Image(feature_matching_video,Theta_max::SMALL, Sclare_rho::SAME,
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

With this function you can either detect lines in a image or track lines in a video. To detect lines we can use mode_capture_photo rather than feature_matching_video. 


The parameter Theta_max::SMALL allows to define the size of the accumulator in the sens of theta (the angle discretized). There are many values : 

```c++
enum class Theta_max : int32_t { XXSMALL = 67 , XSMALL = 127 , SMALL = 255, MEDIUM = 500, LARGE = 1000 , XLARGE = 1500};

```

The parameter Sclare_rho allows to define the scale to use reduce the size of the diagonal of the image used for the accumulator.

```c++
enum class Sclare_rho : int32_t { ONE_QUART = 3, HALF = 2 , THREE_QUART = 1 , SAME = 0};

```


