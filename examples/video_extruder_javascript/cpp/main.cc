#include <iod/timer.hh>

#include <vpp/vpp.hh>
#include <vpp/algorithms/video_extruder/video_extruder.hh>
#include <vpp/draw/draw_trajectories.hh>

#include "symbols.hh"

using namespace vpp;

box2d domain;
int detector_th = 10;
int detector_period = 5;
int keypoint_spacing = 5;
int nframes = 0;

image2d<unsigned char> prev_frame;
image2d<vuchar4> display;
video_extruder_ctx* tr = nullptr;

extern "C"
{
  void set_threshold(int n)
  {
    detector_th = n;
  }

  uint64_t get_display_buffer()
  {
    return uint64_t(display.data());
  }

  uint32_t get_n_points()
  {
    if (!th) return 0;
    else
      return tr->trajectories.size();
  }
  
  void update(int data, unsigned width, unsigned height)
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
    }

    image2d<vuchar4> frame(int(height), int(width), _data = (vuchar4*) data, _pitch = width * 4);
    image2d<unsigned char> frame_gl;

    frame_gl = rgb_to_graylevel<unsigned char>(frame);

    if (nframes > 0)
      video_extruder_update(*tr, prev_frame, frame_gl,
                            _detector_th = detector_th,
                            _keypoint_spacing = keypoint_spacing,
                            _detector_period = detector_period);

    prev_frame = frame_gl;
    image2d<vuchar3> display_tmp;
    pixel_wise(frame, display_tmp) | [] (auto& i, auto& o) { o = i.template segment<3>(0); };
    //copy(frame, display);
    draw::draw_trajectories(display_tmp, tr->trajectories, 200);

    pixel_wise(display_tmp, display) | [] (auto& i, auto& o) {
      o.template segment<3>(0) = i;
      o[3] = 1;
    };
    
    nframes++;
  }

}
