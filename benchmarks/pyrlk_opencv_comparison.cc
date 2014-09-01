#include <iostream>
#include <vpp/vpp.hh>

#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/pyrlk/pyrlk_match.hh>
#include <vpp/algorithms/filters/scharr.hh>
#include <vpp/algorithms/FAST_detector/FAST.hh>

#include <opencv2/opencv.hpp>

using namespace vpp;

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cout << "Usage: " << argv[0] << " image1 image2" << std::endl;
    return 1;
  }

  // image2d<vuchar3> i1 = (from_opencv<vuchar3>(cv::imread(argv[1])));
  // image2d<vuchar1> i3 = rgb_to_graylevel<unsigned char>(i1);
  image2d<vuchar1> i1 = rgb_to_graylevel<unsigned char>(from_opencv<vuchar3>(cv::imread(argv[1])));
  image2d<vuchar1> i2 = rgb_to_graylevel<unsigned char>(from_opencv<vuchar3>(cv::imread(argv[2])));

  i1 = clone_with_border(i1, 5);
  i2 = clone_with_border(i2, 5);
  image2d<vuchar3> display = graylevel_to_rgb<unsigned char>(i1);

  image2d<int> fast_score(i1.domain());
  pyrlk_keypoint_container keypoints(i1.domain());

  fast_detector9(*(image2d<unsigned char>*)&i1, fast_score, 40);
  fast9_scores(*(image2d<unsigned char>*)&i1, fast_score, 40);
  //fast9_filter_localmaximas(fast_score);
  fast9_blockwise_maxima(fast_score, 20);

  std::vector<vint2> pts;
  make_keypoint_vector(fast_score, pts);
  for (vint2 p : pts) keypoints.add(keypoint<float>(p.cast<float>()), 0);

  std::vector<cv::Point2f> cv_keypoints;

  for (auto kp : keypoints.keypoints())
    cv_keypoints.push_back(cv::Point2f(kp.position[1], kp.position[0]));

  int nscales = 4;

  pyramid2d<vuchar1> pyramid1(i1.domain(), nscales, 2, border(3));
  pyramid2d<vuchar1> pyramid2(i1.domain(), nscales, 2, border(3));
  pyramid2d<vfloat2> pyramid1_grad(i1.domain(), nscales, 2, border(3));

  copy(i1, pyramid1[0]);
  copy(i2, pyramid2[0]);


  pyramid1.propagate_level0();
  pyramid2.propagate_level0();
  scharr(pyramid1[0], pyramid1_grad[0]);
  pyramid1_grad.propagate_level0();


  // Vpp Pyrlk.
#define WINDOW_SIZE 11
  pyrlk_match(pyramid1, pyramid1_grad, pyramid2, keypoints, lk_match_point_square_win<WINDOW_SIZE>(), 0.0001, 500, 30, 0.01);

  image2d<vfloat2> vpp_flow(i1.domain());
  fill(vpp_flow, vfloat2{0.f,0.f});
  for (auto& kp: keypoints.keypoints())
    if (kp.age > 0 and vpp_flow.has(kp.position.cast<int>()))
      vpp_flow((kp.position - kp.velocity).cast<int>()) = kp.velocity;

  // Opencv Pyrlk.
  std::vector<cv::Point2f> cv_keypoints2;
  std::vector<unsigned char> status;
  cv::Mat err;
  calcOpticalFlowPyrLK(to_opencv(i1), to_opencv(i2), cv_keypoints, cv_keypoints2, status, err,
                       cv::Size(WINDOW_SIZE, WINDOW_SIZE), nscales);

  image2d<vfloat2> cv_flow(i1.domain());
  fill(cv_flow, vfloat2{0.f,0.f});
  for (int i = 0; i < cv_keypoints.size(); i++)
  {
    vfloat2 a(cv_keypoints[i].y, cv_keypoints[i].x);
    vfloat2 b(cv_keypoints2[i].y, cv_keypoints2[i].x);
    if (cv_flow.has(a.cast<int>()))
        cv_flow(a.cast<int>()) = b - a;
  }

  // Display vectors.
  for (vint2 p : cv_flow.domain())
  {
    float err = ((cv_flow(p) - vpp_flow(p)).norm());
    if (err > 0.5 && cv_flow(p) != vfloat2{0.f, 0.f})
      draw::line2d(display, p, p + cv_flow(p).cast<int>(), vuchar3{255, 0, 0});
    if (err > 0.5 && vpp_flow(p) != vfloat2{0.f, 0.f})
      draw::line2d(display, p, p + vpp_flow(p).cast<int>(), vuchar3{0, 255, 0});

    if (err < 0.5 && vpp_flow(p) != vfloat2{0.f, 0.f})
    {
      draw::c9(display, p, vuchar3{0, 0, 255});
    }
  }

  cv::imwrite("cv_vpp_flow.ppm", to_opencv(display));
}
