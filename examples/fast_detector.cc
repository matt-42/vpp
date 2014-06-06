#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/FAST_detector/FAST.hh>


int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 3)
  {
    std::cerr << "Usage : " << argv[0] << " image threshold" << std::endl;
    return 1;
  }

  typedef image2d<vuchar3> I;
  I A = clone_with_border(from_opencv<vuchar3>(cv::imread(argv[1])), 3);
  I B(A.domain(), 1);

  image2d<unsigned char> Agl(A.domain(), 3);
  image2d<unsigned char> Bgl(A.domain());

  pixel_wise(Agl, A) << [] (unsigned char& gl, vuchar3& c)
  {
    gl = (c[0] + c[1] + c[2]) / 3;
  };

  fast_detector<9>(Agl, Bgl, atoi(argv[2]));

  pixel_wise(Bgl) << [] (unsigned char& gl)
  {
    gl = gl * 255;
  };

  cv::imwrite("b.pgm", to_opencv(Bgl));

}
