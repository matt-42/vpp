#ifndef VPP_OPENCV_BRIDGE_HH__
# define VPP_OPENCV_BRIDGE_HH__

namespace vpp
{

  template <typename V>
  image2d<V> from_opencv(cv::Mat m);

  template <typename V>
  cv::Mat to_opencv(image2d<N> m);

};

#endif
