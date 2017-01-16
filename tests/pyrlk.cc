#include <iostream>
#include <vpp/vpp.hh>

#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/lucas_kanade.hh>
#include <vpp/algorithms/filters/scharr.hh>

#include <opencv2/opencv.hpp>

using namespace vpp;

int main(int argc, char* argv[])
{
  image2d<uint8_t> i1(100,100);
  image2d<uint8_t> i2(100,100);

  image2d<uint8_t> i1_blur(100,100);
  image2d<uint8_t> i2_blur(100,100);

  fill(i1, 0);
  fill(i2, 0);

  draw::square(i1, _center = vint2{50,50}, _width = 5, _fill = 255);
  draw::square(i2, _center = vint2{52,52}, _width = 5, _fill = 255);

  cv::GaussianBlur(to_opencv(i1), to_opencv(i1_blur), cv::Size(9,9), 3, 5, cv::BORDER_REPLICATE);
  cv::GaussianBlur(to_opencv(i2), to_opencv(i2_blur), cv::Size(9,9), 3, 5, cv::BORDER_REPLICATE);

  std::vector<vfloat2> keypoints;
  keypoints.push_back(vfloat2(50, 50));

  if (argc > 1 && std::string(argv[1]) == "--verbose")
  {
    std::cout << "Writing i1.jpg and i2.jpg" << std::endl;
    cv::imwrite("i1.jpg", to_opencv(i1_blur));
    cv::imwrite("i2.jpg", to_opencv(i2_blur));
  }

  lucas_kanade(i1_blur, i2_blur,
  	       _keypoints = keypoints,
  	       _niterations = 50,
  	       _winsize = 5,
  	       _min_ev = 0.001,
  	       _delta = 0.01,
  	       _nscales = 2,
  	       _flow = [] (vfloat2 p, vfloat2 f, int d)
  		 {
  		   assert(p == vfloat2(50.f, 50.f));
  		   assert((f - vfloat2(2.f, 2.f)).norm() < 0.05);
  		 });
}
