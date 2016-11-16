#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/fast_detector/fast.hh>


int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 3)
  {
    std::cerr << "Usage : " << argv[0] << " image threshold" << std::endl;
    return 1;
  }

  image2d<unsigned char> A = rgb_to_graylevel<unsigned char>(from_opencv<vuchar3>(cv::imread(argv[1])));
  A = clone(A, _border = 3);

  std::vector<vint2> keypoints = fast9(A, atoi(argv[2]), _local_maxima);

  image2d<vuchar3> B = graylevel_to_rgb<vuchar3>(A);
  for (vint2 p : keypoints) B(p) = vuchar3(0,0,255);
  
  cv::imwrite("b.ppm", to_opencv(B));
}
