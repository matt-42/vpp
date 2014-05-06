#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/opencv_bridge.hh>


int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 3)
  {
    std::cerr << "Usage : " << argv[0] << " image" << std::endl;
    return 1;
  }

  typedef image2d<vuchar3> I;
  I A = clone_with_border(from_opencv<vuchar3>(cv::imread(argv[1])), 1);
  I B(A.domain(), 1);

  window<I> nbh = window<I>(A, { {0, -1}, {0, 0}, {0, 1} });

  // Parallel Loop over pixels of in and out.
  pixel_wise(A, B) << [&] (auto& a, auto& b) {
    vint3 sum = vint3::Zero();

    // Loop over in's neighboords wrt nbh to compute a sum.
    for (vuchar3& n : nbh(a)) sum += n.cast<int>();

    // Write the sum to the output image.
    b = (sum / 3).cast<unsigned char>();
  };

  cv::imwrite("b.ppm", to_opencv(B));

}
