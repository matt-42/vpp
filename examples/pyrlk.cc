#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/FAST_detector/FAST.hh>
#include <vpp/algorithms/pyrlk/pyrlk_match.hh>
#include <vpp/algorithms/filters/scharr.hh>


#include <vpp/core/dige.hh>
#include <dige/window.h>
#include <dige/image.h>
#include <dige/widgets/image_view.h>


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
  using dg::widgets::ImageView;
  using dg::widgets::show;
  using dg::widgets::newline;

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

  pyramid2d<vuchar1> pyramid1(frame.domain(), nscales, 2, border(3));
  pyramid2d<vuchar1> pyramid2(frame.domain(), nscales, 2, border(3));

  pyramid2d<vfloat2> pyramid1_grad(frame.domain(), nscales, 2, border(3));

  image2d<int> detector(domain, 10);

  pyrlk_keypoint_container keypoints(domain);
  typedef pyrlk_keypoint_container::keypoint_type KP;

  image2d<vuchar1> grad_display(domain);
  image2d<vuchar3> frame_display(domain);

  while (cap.read(cvframe))
  {
    pixel_wise(frame, pyramid1[0]) << [] (const vuchar3& in, vuchar1& out) { out[0] = (in[0] + in[1] + in[2]) / 3; };
    pyramid1.propagate_level0();
    scharr(pyramid1[0], pyramid1_grad[0]);
    pyramid1_grad.propagate_level0();

    // Compute the keypoint mask.
    fill(detector, 1);
    block_wise(vint2(10, 10), detector, keypoints.index2d()) << [] (image2d<int> D, image2d<int> I)
    {
      if (sum(I) > 0) fill(D, 0);
    };

    fast9_detect(*(image2d<unsigned char>*)&(pyramid1[0]), detector, fast_th);
    fast9_scores(*(image2d<unsigned char>*)&(pyramid1[0]), detector, fast_th);
    //fast9_filter_localmaximas(detector);
    fast9_blockwise_maxima(detector, 10);
    std::vector<vint2> kps;
    make_keypoint_vector(detector, kps);

    int f;
    for (vint2 p : kps) keypoints.add(KP(p), f);
    pyrlk_match(pyramid1, pyramid1_grad, pyramid2, keypoints, lk_match_point_square_win<5>(), 0.01, 3);


    copy(frame, frame_display);
    auto rect = box_nbh2d<vuchar3, 3, 3>(frame);
    for (vint2 p : kps)
      rect(frame(p)) < [] (vuchar3& n) { n = vuchar3(0, 0, 255); };

    pixel_wise(pyramid1_grad[0], grad_display) << [] (vfloat2 g, vuchar1& out) { out[0] = 128 + g[0]; };

    ImageView("frame") << frame << grad_display << show;

    std::cout << frame_cpt << " " << keypoints.size() << std::endl;
    frame_cpt++;


    pyramid1.swap(pyramid2);
  }

}
