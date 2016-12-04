#include <opencv2/highgui.hpp>

#include <iod/parse_command_line.hh>
#include <iod/timer.hh>

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
                                 cl::positionals(_video, _detector_th, _keypoint_spacing),
                                 cl::required(_video),
                                 _video = std::string(),
                                 _detector_th = int(10),
                                 _keypoint_spacing = int(5),
                                 _record_video = std::string());

  box2d domain = videocapture_domain(opts.video.c_str());
  video_extruder_ctx ctx = video_extruder_init(domain);

  cv::VideoWriter output_video;
  if (opts.record_video.size())
  {
    output_video.open(opts.record_video, cv::VideoWriter::fourcc('M','J','P','G'), 30.f,
                      cv::Size(domain.ncols(), domain.nrows()), true);
  }
  
  image2d<unsigned char> prev_frame(domain);

  bool first = true;
  int nframes = 0;

  int us_cpt = 0;
  foreach_videoframe(opts.video.c_str()) | [&] (const image2d<vuchar3>& frame)
  {
    auto frame_gl = rgb_to_graylevel<unsigned char>(frame);
    timer t;
    t.start();
    if (!first)
      video_extruder_update(ctx, prev_frame, frame_gl,
                            _detector_th = opts.detector_th,
                            _keypoint_spacing = opts.keypoint_spacing,
                            _detector_period = 5,
                            _max_trajectory_length = 100);
    else first = false;
    t.end();

    us_cpt += t.us();
    if (!(nframes%1000))
    {
        std::cout << "Tracker time: " << (us_cpt / 1000000.f) << " ms/frame. " << ctx.trajectories.size() << " particles." << std::endl;
        us_cpt = 0;
    }

    vpp::copy(frame_gl, prev_frame);
    auto display = clone(frame);
    draw::draw_trajectories(display, ctx.trajectories, 200);
    cv::imshow("Trajectories", to_opencv(display));
    cv::waitKey(1);

    if (output_video.isOpened())
      output_video << to_opencv(display);
    
    nframes++;
  };
  
}
