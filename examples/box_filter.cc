#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>


int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 2)
  {
    std::cerr << "Usage : " << argv[0] << " image" << std::endl;
    return 1;
  }

  typedef image2d<vuchar3> I;
  I A = clone(from_opencv<vuchar3>(cv::imread(argv[1])), vpp::_border = 1);
  I B(A.domain());

  auto nbh = box_nbh2d<vuchar3, 3, 3>(A);

  // Parallel Loop over pixels of in and out.
  pixel_wise(nbh, B) | [] (auto& n, auto& b) {
    vint3 sum = vint3::Zero();

    sum += n(0, -1).template cast<int>();
    sum += n(0, 0).template cast<int>();
    sum += n(0, 1).template cast<int>();

    // Write the sum to the output image.
    b = (sum / 3).cast<unsigned char>();
  };

  cv::imwrite("b.ppm", to_opencv(B));

}
