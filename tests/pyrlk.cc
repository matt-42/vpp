#include <iostream>
#include <vpp/vpp.hh>

#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/pyrlk/pyrlk_match.hh>
#include <vpp/algorithms/filters/scharr.hh>

#include <opencv2/opencv.hpp>

using namespace vpp;

int main(int argc, char* argv[])
{
  image2d<vuchar1> i1(100,100);
  image2d<vuchar1> i2(100,100);

  image2d<vuchar1> i1_blur(100,100);
  image2d<vuchar1> i2_blur(100,100);

  fill(i1, vuchar1::Zero());
  fill(i2, vuchar1::Zero());

  box_nbh2d<vuchar1, 5, 5> box(i1);

  box_nbh2d<vuchar1, 5, 5>(i1, vint2(50,50)).for_all([] (auto& n) { n[0] = 255; });
  box_nbh2d<vuchar1, 5, 5>(i1, vint2(52,52)).for_all([] (auto& n) { n[0] = 255; });

  // box(i1(50,50)) < [] (auto& n) { n[0] = 255; };
  // box(i2(52,52)) < [] (auto& n) { n[0] = 255; };

  cv::GaussianBlur(to_opencv(i1), to_opencv(i1_blur), cv::Size(9,9), 3, 5, cv::BORDER_REPLICATE);
  cv::GaussianBlur(to_opencv(i2), to_opencv(i2_blur), cv::Size(9,9), 3, 5, cv::BORDER_REPLICATE);

  pyrlk_keypoint_container keypoints(i1.domain());
  int f;
  keypoints.add(keypoint<float>(vfloat2(50, 50)), f);

  int nscales = 4;

  pyramid2d<vuchar1> pyramid1(i1.domain(), nscales, 2, _border = 3);
  pyramid2d<vuchar1> pyramid2(i1.domain(), nscales, 2, _border = 3);
  pyramid2d<vfloat2> pyramid1_grad(i1.domain(), nscales, 2, _border = 3);

  copy(i1_blur, pyramid1[0]);
  copy(i2_blur, pyramid2[0]);


  pyramid1.propagate_level0();
  pyramid2.propagate_level0();

  scharr(pyramid1[0], pyramid1_grad[0]);

  pyramid1_grad.propagate_level0();


  if (argc > 1 && std::string(argv[1]) == "--verbose")
  {
    std::cout << "Writing i1.jpg and i2.jpg" << std::endl;
    cv::imwrite("i1.jpg", to_opencv(i1_blur));
    cv::imwrite("i2.jpg", to_opencv(i2_blur));

    cv::imwrite("i1_3.jpg", to_opencv(pyramid1[3]));
    cv::imwrite("i2_3.jpg", to_opencv(pyramid2[3]));
  }

  pyrlk_match(pyramid1, pyramid1_grad, pyramid2, keypoints, lk_match_point_square_win<5>(), 0.01, 50, 50, 0.001);

  assert(keypoints.size() == 1);
  std::cout << keypoints[0].position.transpose() << std::endl;
  assert((keypoints[0].position - vfloat2(52,52)).norm() < 0.2);
}
