#ifndef VPP_OPENCV_UTILS_HH_
# define VPP_OPENCV_UTILS_HH_

# include <iostream>
# include <regex>
# include <opencv2/highgui/highgui.hpp>
# include <vpp/core/boxNd.hh>
# include <vpp/core/image2d.hh>

inline bool open_videocapture(const char* str, cv::VideoCapture& cap)
{
  if (std::regex_match(str, std::regex("[0-9]+")))
    cap.open(atoi(str));
  else cap.open(str);

  if (!cap.isOpened())
  {
    std::cerr << "Error: Cannot open " << str << std::endl;
    return false;
  }

  return true;
}

inline vpp::box2d videocapture_domain(cv::VideoCapture& cap)
{
  return vpp::make_box2d(cap.get(CV_CAP_PROP_FRAME_HEIGHT),
                    cap.get(CV_CAP_PROP_FRAME_WIDTH));
}

inline vpp::box2d videocapture_domain(const char* f)
{
  cv::VideoCapture cap;
  open_videocapture(f, cap);
  return vpp::make_box2d(cap.get(CV_CAP_PROP_FRAME_HEIGHT),
                    cap.get(CV_CAP_PROP_FRAME_WIDTH));
}

struct foreach_videoframe
{

  foreach_videoframe(const char* f)
  {
    open_videocapture(f, cap_);
    frame_ = vpp::image2d<vpp::vuchar3>(videocapture_domain(cap_));
    cvframe_ = to_opencv(frame_);
  }
  
  template <typename F>
  void operator| (F f)
  {
    while (cap_.read(cvframe_)) f(frame_);
  }
    
private:
  cv::Mat cvframe_;
  vpp::image2d<vpp::vuchar3> frame_;
  cv::VideoCapture cap_;
};
#endif
