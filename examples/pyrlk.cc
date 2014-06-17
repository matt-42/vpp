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

#include <regex>

using namespace vpp;

bool open_videocapture(const char* str, cv::VideoCapture& cap)
{
  if (std::regex_match(str, std::regex("[0-9]+")))
    cap.open(atoi(str));
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

  if (argc != 5)
  {
    std::cerr << "Usage : " << argv[0] << " video nscales fast_threshold box_size" << std::endl;
    return 1;
  }

  cv::VideoCapture cap;
  if (!open_videocapture(argv[1], cap)) return 1;

  int nscales = atoi(argv[2]);
  int fast_th = atoi(argv[3]);
  int box_size = atoi(argv[4]);

  int frame_cpt = 0;

  box2d domain = videocapture_domain(cap);
  image2d<vuchar3> frame(domain);
  image2d<vuchar1> framegl(domain);
  cv::Mat cvframe = to_opencv(frame);

  pyramid2d<vuchar1> pyramid_next(frame.domain(), nscales, 2, border(3));
  pyramid2d<vuchar1> pyramid_prev(frame.domain(), nscales, 2, border(3));
  pyramid2d<vfloat2> pyramid_prev_grad(frame.domain(), nscales, 2, border(3));

  image2d<int> detector(domain, 10);

  pyrlk_keypoint_container keypoints(domain);
  typedef pyrlk_keypoint_container::keypoint_type KP;

  image2d<vuchar1> grad_display(domain);
  image2d<vuchar3> frame_display(domain);

  while (cap.read(cvframe))
  {
    pixel_wise(frame, pyramid_next[0]) << [] (const vuchar3& in, vuchar1& out) { out[0] = (in[0] + in[1] + in[2]) / 3; };
    pyramid_next.propagate_level0();
    scharr(pyramid_prev[0], pyramid_prev_grad[0]);
    pyramid_prev_grad.propagate_level0();

    if (!(frame_cpt % 5))
    {
      fill(detector, 1);
      fast9_detect(*(image2d<unsigned char>*)&(pyramid_next[0]), detector, fast_th);
      fast9_scores(*(image2d<unsigned char>*)&(pyramid_next[0]), detector, fast_th);
      fast9_blockwise_maxima(detector, box_size);
      block_wise(vint2(box_size, box_size), detector, keypoints.index2d())
        << [box_size] (image2d<int> D, image2d<int> I)
      {
        if (sum(I) > -(box_size*box_size)) fill(D, 0);
      };

      std::vector<vint2> kps;
      make_keypoint_vector(detector, kps);

      int f;
      for (vint2 p : kps) keypoints.add(KP(cast<vfloat2>(p)), f);
    }

    pyrlk_match(pyramid_prev, pyramid_prev_grad, pyramid_next, keypoints, lk_match_point_square_win<11>(), 0.01, 10);

    keypoints.compact();

    copy(frame, frame_display);
    auto rect = box_nbh2d<vuchar3, 3, 3>(frame);

    for (auto& p : keypoints.keypoints())
      if (p.age > 2)
        rect(frame(p.position.cast<int>())) < [] (vuchar3& n) { n = vuchar3(0, 0, 255); };

    ImageView("frame") << frame << show;

    std::cout << "Frame: "<< frame_cpt << " " << keypoints.size() << " keypoints." << std::endl;
    frame_cpt++;


    pyramid_next.swap(pyramid_prev);
  }

}
