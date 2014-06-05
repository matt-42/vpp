#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/FAST_detector/FAST.hh>
#include <vpp/algorithms/pyrlk/pyrlk_match.hh>
#include <vpp/algorithms/filters/scharr.hh>

using namespace vpp;

bool open_videocapture(const char* str, cv::VideoCapture& cap)
{
  long int device = atoi(str);
  if (device > 0)
    cap.open(device);
  else cap.open(str);

  if (!cap.isOpened())
  {
    std::cerr << "Error: Cannot open " << str << std::endl;
    return false;
  }

  return true;
}

box2d videocapture_domain(cv::VideoCapture& cap)
{
  return make_box2d(cap.get(CV_CAP_PROP_FRAME_HEIGHT),
                    cap.get(CV_CAP_PROP_FRAME_WIDTH));
}

int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 4)
  {
    std::cerr << "Usage : " << argv[0] << " video nscales fast_threshold" << std::endl;
    return 1;
  }

  cv::VideoCapture cap;
  if (!open_videocapture(argv[1], cap)) return 1;

  int nscales = atoi(argv[2]);
  int fast_th = atoi(argv[3]);

  int frame_cpt = 0;

  box2d domain = videocapture_domain(cap);
  image2d<vuchar3> frame(domain);
  image2d<vuchar1> framegl(domain);
  cv::Mat cvframe = to_opencv(frame);

  pyramid2d<vuchar1> pyramid1(frame.domain(), nscales, 2);
  pyramid2d<vuchar1> pyramid2(frame.domain(), nscales, 2);

  pyramid2d<vfloat2> pyramid1_grad(frame.domain(), nscales, 2);

  pyrlk_keypoint_container keypoints(domain);
  while (cap.read(cvframe))
  {
    pixel_wise(frame, pyramid1[0]) << [] (const vuchar3& in, vuchar1& out) { out[0] = (in[0] + in[1] + in[2]) / 3; };
    pyramid1.propagate_level0();

    scharr(pyramid1[0], pyramid1_grad[0]);
    pyramid1_grad.propagate_level0();

    pyrlk_match(pyramid1, pyramid1_grad, pyramid2, keypoints, lk_match_point_square_win<5>(), 0.01, 3);

    std::cout << frame_cpt << std::endl;
    frame_cpt++;

    pyramid1.swap(pyramid2);
  }

}
