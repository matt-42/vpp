#include <opencv2/highgui.hpp>

#include <iod/parse_command_line.hh>

#include <vpp/vpp.hh>
#include <vpp/algorithms/video_extruder/video_extruder.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/utils/opencv_utils.hh>
#include <vpp/draw/draw_trajectories.hh>

#include "symbols.hh"

using namespace iod;
using namespace vpp;

int main(int argc, const char* argv[])
{
  
  auto opts = parse_command_line(argc, argv,
                                 cl::positionals(_video),
                                 cl::required(_video),
                                 _video = std::string(),
                                 _detector_th = int(10));

  box2d domain = videocapture_domain(opts.video.c_str());
  video_extruder_ctx ctx = video_extruder_init(domain);

  image2d<unsigned char> prev_frame(domain);

  bool first = true;
  foreach_videoframe(opts.video.c_str()) | [&] (const image2d<vuchar3>& frame)
  {
    auto frame_gl = rgb_to_graylevel<unsigned char>(frame);
    if (!first)
      video_extruder_update(ctx, frame_gl, prev_frame, _detector_th = opts.detector_th);
    else first = false;

    vpp::copy(frame_gl, prev_frame);

    auto display = clone(frame);
    draw::draw_trajectories(display, ctx.trajectories);
    cv::imshow("Trajectories", to_opencv(display));
  };
  
}
