#include <opencv2/highgui.hpp>

#include <iod/parse_command_line.hh>
#include <iod/timer.hh>

#include <vpp/algorithms/symbols.hh>

#include <vpp/vpp.hh>
#include <vpp/algorithms/video_extruder.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/utils/opencv_utils.hh>

#include "symbols.hh"

using namespace iod;
using namespace vpp;
using namespace s;

void paint(std::vector<keypoint_trajectory>& trs,
           image2d<vuchar4>& paint_buffer)
{
  // Decrease opacity.
  pixel_wise(paint_buffer) | [] (vuchar4& v) {
    v[3] *= 0.97;
  };

#pragma omp parallel for
  for (int i = 0; i < trs.size(); i++)
  {
    auto t = trs[i];
    if (t.alive() and t.size() > 1)
    {
      vint2 p1 = t.position_at_frame(t.end_frame()).template cast<int>();
      vint2 p2 = t.position_at_frame(t.end_frame() - 1).template cast<int>();
      vint2 p3 = t.position_at_frame(t.end_frame() - std::min(t.size() - 1, 10)).template cast<int>();

      float speed = (p3 - p1).norm();
      vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p3[0] - p1[0], p3[1] - p1[1])) * 180.f / M_PI,
                                    1.f, 1.f);

      float alpha = std::min(1.f, speed / 10);

      auto paint = [&] (vint2 x, float a)
        {
          if (!paint_buffer.has(x)) return;
          vuchar4 c;
          c.template segment<3>(0) = pt_color;
          c[3] = 255*1*alpha;
          vpp::draw::plot_color(paint_buffer, x, c);
        };

      auto paint2 = [&] (vint2 x, float a, int d)
        {
          if (!paint_buffer.has(x)) return;
          vuchar4 c;
          c.template segment<3>(0) = pt_color;
          c[3] = 255*0.7*alpha;
          vpp::draw::plot_color(paint_buffer, x, c);
        };

      if ((p1 - p3).norm() > 5)
        draw::line2d(p1, p2, paint, paint2);
    }
  }
}

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

  image2d<unsigned char> prev_frame(domain, _border = 3);
  image2d<vuchar4> paint_buffer(domain);
  fill(paint_buffer, vuchar4(0,0,0,0));
  
  bool first = true;
  int nframes = 0;

  cv::VideoWriter output_video;
  if (opts.record_video.size())
  {
    output_video.open(opts.record_video, cv::VideoWriter::fourcc('M','J','P','G'), 30.f,
                      cv::Size(domain.ncols(), domain.nrows()), true);
  }

  int us_cpt = 0;
  foreach_videoframe(opts.video.c_str()) | [&] (const image2d<vuchar3>& frame_cv)
  {
    auto frame = clone(frame_cv, _border = 3);
    fill_border_mirror(frame);
    auto frame_gl = rgb_to_graylevel<unsigned char>(frame);
    timer t;
    t.start();
    if (!first)
      video_extruder_update(ctx, prev_frame, frame_gl,
                            _detector_th = opts.detector_th,
                            _keypoint_spacing = opts.keypoint_spacing,
                            _detector_period = 5,
                            _nscales = 3,
                            _winsize = 9,
                            _patchsize = 5,
                            _propagation = 2);
    else first = false;
    t.end();

    us_cpt += t.us();
    if (!(nframes%500))
    {
        std::cout << "Tracker time: " << (us_cpt / 1000000.f) << " ms/frame. " << ctx.trajectories.size() << " particles." << std::endl;
        us_cpt = 0;
    }

    vpp::copy(frame_gl, prev_frame);
    auto display = clone(frame);
    paint(ctx.trajectories, paint_buffer);

    pixel_wise(paint_buffer, display) | [] (auto p, auto& d)
    {
      d = ((d.template cast<int>() * (255 - p[3]) + p.template segment<3>(0).template cast<int>() * p[3]) / 255).template cast<unsigned char>();
    };
    //draw::draw_trajectories(display, ctx.trajectories, 200);
    cv::imshow("Trajectories", to_opencv(display));
    cv::waitKey(1);

    if (output_video.isOpened())
      output_video << to_opencv(display);
    nframes++;
  };

  output_video.release();
}
