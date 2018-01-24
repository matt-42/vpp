

A fast embedded implementation of a line detection algorithm using a dense one-to-one Hough transform and a tracking through the Hough space.

## Getting Started

This project contains a fast Dense one-to-one Hough transform based on Video++ library. There is also a line tracking method, to temporally associate every line to its successive positions along the video. Our method then performs mode (cluster) tracking in the Hough space, using prediction and matching of the clusters based on both their position and appearance. The implementation takes advantage of the high performance video processing library Video++, that allows to parallelise simply and efficiently many video primitives.  
We have implement a method to detect vanishing points and the RPLN method (Robuts Pose Estimation from Lines Correspondences) http://www.mip.informatik.uni-kiel.de/tiki-index.php?page=Lilian+Zhang

### Prerequisites

The prerequisites are the same as for Video++. But you must include unsupported eigen modules.


### Installing

The program is furnished as headers so it is enough to install Video++. There a demo of the tracking here :
https://www.dropbox.com/s/bh980ucuxcmci40/original_video.avi?dl=0

## Example

You can launch the program by calling :

```c++
    fast_dht_matching(feature_matching,Theta_max::SMALL, Sclare_rho::SAME,
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

The parameter Type_output is used to define the type of video to output after the tracking

```c++
enum class Type_output : int16_t { VIDEO_HOUGH = 0 , ORIGINAL_VIDEO = 1 , GRADIENT_VIDEO = 2,ORIGINAL_AND_HOUGH_VIDEO = 3   };

```


The parameter Type_video_hough is used to define the type of output of tracking video.

```c++
enum class Type_video_hough : int8_t { ONLY_CLUSTERS = 1 , ALL_POINTS = 2 };

```

The parameter Frequence is used to set if the dense hough transform is performed through all frames or only in certain frames.

```c++
enum class Frequence : int8_t { ALL_FRAME = 0 , NOT_ALL = 1 };

```

The parameter With_Kalman_Filter expresses the fact if a Kalman is used to predict the position of clusters in such a way that even if a line does not appear in certain frames, this line next positions of this line will be predict and when it will reappear the tracking will continue. Here we use an unscented kalman filter. _nombre_max_frame_without_update defines the sucessive number of frames we keep a line in the list even if it is not visible in the field of view.

```c++
enum class With_Kalman_Filter : int8_t { YES = 1 , NO = 0 };

```

The parameter With_Transparency is used to set if for the output video, we'll use or not the alpha transparency to draw lines trajectories.

```c++
enum class With_Transparency : int8_t { YES = 1 , NO = 0 };
```

The parameter With_Entries is used to take into account of entries or not during the tracking.

```c++
enum class With_Entries : int8_t { YES = 1 , NO = 0 };

```





