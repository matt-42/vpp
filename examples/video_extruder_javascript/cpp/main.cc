#include <iod/timer.hh>

#include <vpp/vpp.hh>
#include <vpp/algorithms/video_extruder/video_extruder.hh>
#include <vpp/draw/draw_trajectories.hh>

#include "symbols.hh"

using namespace vpp;

box2d domain = make_box2d(0,0);
int detector_th = 5;
int detector_period = 15;
int keypoint_spacing = 11;
int nframes = 0;

image2d<unsigned char> prev_frame;
image2d<vuchar4> display;
image2d<vuchar4> paint_buffer;
video_extruder_ctx* tr = nullptr;

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

      float alpha = std::min(1.f, speed / 20);

      auto paint = [&] (vint2 x, float it)
        {
          if (!paint_buffer.has(x)) return;
          vuchar4 c;
          c.template segment<3>(0) = pt_color;
          c[3] = 255*1*alpha;
          vpp::draw::plot_color(paint_buffer, x, c);
        };

      auto paint2 = [&] (vint2 x, float it, int d)
        {
          if (!paint_buffer.has(x)) return;
          vuchar4 c;
          c.template segment<3>(0) = pt_color;
          c[3] = 255*alpha*(5.f-d)/5.f;
          vpp::draw::plot_color(paint_buffer, x, c);
        };

      if ((p1 - p3).norm() > 5)
        draw::line2d(p1, p2, paint, paint2);
    }
  }
}

extern "C"
{
  void set_threshold(int n)
  {
    detector_th = n;
  }

  int get_display_buffer()
  {
    if (display.has_data())
      return int(display.data());
    else
      return 0;
  }

  int get_n_points()
  {
    if (!tr) return 0;
    else
      return tr->trajectories.size();
  }
  
  int update(int data, int width, int height)
  {
    if (!tr || domain.nrows() != height || domain.ncols() != width)
    {
      if (tr) delete tr;
      domain = make_box2d(height, width);

      tr = new video_extruder_ctx(domain);

      *tr = video_extruder_init(domain);
      nframes = 0;
      prev_frame = image2d<unsigned char>(domain);
      display = image2d<vuchar4>(domain);
      paint_buffer = image2d<vuchar4>(domain);
      fill(paint_buffer, vuchar4(0,0,0,0));

    }

    image2d<vuchar4> frame(int(height), int(width), _data = (vuchar4*) data, _pitch = width * 4);
    image2d<unsigned char> frame_gl;

    // for (int r = 0; r < frame.nrows(); r++)
    // for (int c = 0; c < frame.nrows(); c++)
    // for (int r = 0; r < frame.nrows(); r++)
    // for (int c = 0; c < frame.ncols(); c++)
    //   frame[r][c] = vuchar4(0,0,0,0);
    frame_gl = rgb_to_graylevel<unsigned char>(frame);

    if (nframes > 0)
      video_extruder_update(*tr, prev_frame, frame_gl,
                            _detector_th = detector_th,
                            _keypoint_spacing = keypoint_spacing,
                            _detector_period = detector_period);

    
    prev_frame = frame_gl;
    // image2d<vuchar3> display_tmp(domain);
    // pixel_wise(frame, display_tmp) | [] (auto& i, auto& o) { o = i.template segment<3>(0); };
    // //copy(frame, display);
    // draw::draw_trajectories(display_tmp, tr->trajectories, 200);

    // pixel_wise(display_tmp, display) | [] (auto& i, auto& o) {
    //   o.template segment<3>(0) = i;
    //   o[3] = 1;
    // };

    vpp::copy(frame_gl, prev_frame);
    //auto display = clone(frame);
    paint(tr->trajectories, paint_buffer);

    copy(frame, display);
    pixel_wise(paint_buffer, display) | [] (auto p, auto& d)
    {
      d.template segment<3>(0) = ((d.template segment<3>(0).template cast<int>() * (255 - p[3]) + p.template segment<3>(0).template cast<int>() * p[3]) / 255).template cast<unsigned char>();
      d[3] = 255;
    };
    
    nframes++;

    return 42;
  }

}
