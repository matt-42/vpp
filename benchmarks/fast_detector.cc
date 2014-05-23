#include <chrono>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/FAST_detector/FAST.hh>
#include <vpp/algorithms/FAST_detector/FAST2.hh>

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
  image2d<int> tmp(A.domain());

  pixel_wise(Agl, A) << [] (unsigned char& gl, vuchar3& c)
  {
    gl = (c[0] + c[1] + c[2]) / 3;
  };

  int th = atoi(argv[2]);
  auto time = get_time_in_seconds();
  std::vector<vint2> keypoints_vpp;
  for (unsigned i = 0; i < K; i++)
  {
    keypoints_vpp.clear();
    fast_detector9(Agl, tmp, th);
    keypoints_to_vector(tmp, keypoints_vpp);
  }
  //fast_detector<9>(Agl, Bgl, th);
  double vpp_ms_per_iter = 1000 * (get_time_in_seconds() - time) / K;

  int vpp_n_keypoints = 0;
  // for (int r = 0; r < Bgl.nrows(); r++)
  // for (int c = 0; c < Bgl.ncols(); c++)
  //   vpp_n_keypoints += Bgl(r, c);

  pixel_wise(tmp) < [&] (auto& b)
  {
    vpp_n_keypoints += b;
  };

  // Opencv: void FAST(InputArray _img, std::vector<KeyPoint>& keypoints, int threshold, bool nonmax_suppression)
  time = get_time_in_seconds();
  cv::Mat cv_Agl = to_opencv(Agl);
  cv::Mat cv_B = to_opencv(B);
  std::vector<cv::KeyPoint> keypoints;
  for (unsigned i = 0; i < K; i++)
  {
    keypoints.clear();
    FAST(cv_Agl, keypoints, th, false);
  }

  std::cout << keypoints.size() << std::endl;

  double opencv_ms_per_iter = 1000 * (get_time_in_seconds() - time) / K;

  std::cout << "vpp: " << vpp_n_keypoints << " keypoints" << std::endl;
  std::cout << "vpp: " << keypoints_vpp.size() << " keypoints" << std::endl;
  std::cout << "openmp: " << keypoints.size() << " keypoints" << std::endl;

  std::cout << "Time per iterations: " << std::endl
            << "OpenCV: " << opencv_ms_per_iter << "ms" << std::endl
            << "VPP: " << vpp_ms_per_iter << "ms" << std::endl;

  pixel_wise(Bgl) << [] (unsigned char& gl)
  {
    gl = gl * 255;
  };

  image2d<vuchar3> out(A.domain());
  pixel_wise(out, Bgl, A) << [] (vuchar3& o, unsigned char& k, vuchar3& in)
  {
    if (!k)
      o = in;
    else
      o = vuchar3(0,0, 255);
  };

  for (int i = 0; i < keypoints.size(); i++)
  {
    //std::cout << "dupp"<< std::endl;
    //Point pt = 
    if (out(keypoints[i].pt.y, keypoints[i].pt.x) == vuchar3(0,0,255))
      A(keypoints[i].pt.y, keypoints[i].pt.x) = vuchar3(0,255,0);
    else
      A(keypoints[i].pt.y, keypoints[i].pt.x) = vuchar3(0,0, 255);
  }

  cv::imwrite("b.pgm", to_opencv(Bgl));
  cv::imwrite("out.ppm", to_opencv(out));
  cv::imwrite("A.ppm", to_opencv(A));

}
