#include <chrono>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/FAST_detector/FAST.hh>

#include "get_time.hh"

int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 3)
  {
    std::cerr << "Usage : " << argv[0] << " image threshold" << std::endl;
    return 1;
  }

  int K = 400;

  typedef image2d<vuchar3> I;
  I A = clone_with_border(from_opencv<vuchar3>(cv::imread(argv[1])), 3);
  I B(A.domain(), 1);

  image2d<unsigned char> Agl(A.domain(), 3);
  image2d<unsigned char> Bgl(A.domain());

  pixel_wise(Agl, A) << [] (unsigned char& gl, vuchar3& c)
  {
    gl = (c[0] + c[1] + c[2]) / 3;
  };

  auto time = get_time_in_seconds();
  for (unsigned i = 0; i < K; i++)
    fast_detector<9>(Agl, Bgl, atoi(argv[2]));
  double vpp_ms_per_iter = 1000 * (get_time_in_seconds() - time) / K;


  // Opencv: void FAST(InputArray _img, std::vector<KeyPoint>& keypoints, int threshold, bool nonmax_suppression)
  time = get_time_in_seconds();
  cv::Mat cv_Agl = to_opencv(Agl);
  cv::Mat cv_B = to_opencv(B);
  std::vector<cv::KeyPoint> keypoints;
  for (unsigned i = 0; i < K; i++)
  {
    keypoints.clear();
    FAST(cv_Agl, keypoints, 10, false);
  }
  double opencv_ms_per_iter = 1000 * (get_time_in_seconds() - time) / K;

  std::cout << "Time per iterations: " << std::endl
            << "OpenCV: " << opencv_ms_per_iter << "ms" << std::endl
            << "VPP: " << vpp_ms_per_iter << "ms" << std::endl;

  pixel_wise(Bgl) << [] (unsigned char& gl)
  {
    gl = gl * 255;
  };

  cv::imwrite("b.pgm", to_opencv(Bgl));

}
