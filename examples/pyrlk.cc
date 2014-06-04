#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/FAST_detector/FAST.hh>
//#include <vpp/algorithms/pyrlk/pyrlk_match.hh>

using namespace vpp;


template <typename F, typename G>
void for_each_video_frames(const char* str, F fun_init, G fun_loop)
{

  cv::VideoCapture cap;

  long int device = atoi(str);
  if (device > 0)
    cap.open(device);
  else cap.open(str);

  if (!cap.isOpened())
  {
    std::cerr << "Error: Cannot open " << str << std::endl;
    return;
  }

  image2d<vuchar3> frame(cap.get(CV_CAP_PROP_FRAME_HEIGHT),
                         cap.get(CV_CAP_PROP_FRAME_WIDTH));

  fun_init(frame.domain());

  std::cout << "Video size: " << frame.nrows() << " " << frame.ncols() << std::endl;

  cv::Mat cvframe = to_opencv(frame);

  int frame_cpt = 0;
  while (cap.read(cvframe))
    fun_loop(frame);
}

int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 4)
  {
    std::cerr << "Usage : " << argv[0] << " video nscales fast_threshold" << std::endl;
    return 1;
  }

  int frame_cpt = 0;

  for_each_video_frames(

    argv[1],

    // Init
    [&] (const box2d& domain)
    {
      std::cout << domain.size(0) << "x" << domain.size(1) << std::endl;
    },

    // Loop
    [&] (const image2d<vuchar3>& frame)
    {
      std::cout << frame_cpt << std::endl;
      frame_cpt++;
    }

    );

}

