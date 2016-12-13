#include <opencv2/highgui.hpp>

#include <iod/parse_command_line.hh>
#include <iod/timer.hh>

#include <gpof/gpof_ios.hh>

#include <vpp/vpp.hh>
#include <vpp/algorithms/video_extruder.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/utils/opencv_utils.hh>
#include <vpp/draw/draw_trajectories.hh>
#include <evaluation/utils/kitti.hh>

#include "symbols.hh"

using namespace iod;
using namespace vpp;


struct stats
{
  stats() : min_(FLT_MAX), max_(FLT_MIN),
            cpt_(0.f), sum_(0.f) {}

  void take(float f)
  {
    min_ = std::min(min_, f);
    max_ = std::max(max_, f);
    cpt_++;
    sum_ += f;
  }

  float min() const { return min_; }
  float max() const { return max_; }
  float cpt() const { return cpt_; }
  float sum() const { return sum_; } 
  float avg() const { return sum_ / cpt_; }
 
  float min_, max_, cpt_, sum_;
};


inline auto display_flow(const image2d<vfloat3>& flow, std::string path)
{
  image2d<vuchar3> rgb_flow(flow.domain());
  fill(rgb_flow, vuchar3{0,0,0});
  pixel_wise(rgb_flow, flow) | [] (vuchar3& rf, vfloat3 f3)
  {
    vfloat2 f = f3.segment<2>(0);
    if (f.norm() != 0.f)
    {
      float a = (M_PI + atan2(f[0], f[1])) * 180 / M_PI;
      float n = std::min(f.norm() / 30.f, 1.f);
      rf = hsv_to_rgb(a, n, 1.);
    }
  };

  cv::imwrite(path, to_opencv(rgb_flow));  
}

void display_flow_errors(const image2d<vfloat3>& flow,
                         const image2d<vfloat3>& ref_flow,
                         const image2d<vuchar3>& i1,
                         std::string path)
{
  image2d<vuchar3> display(i1.domain());
  copy(i1, display);
  for (vint2 p : flow.domain())
  {
    if (!flow(p)[2]) continue;

    vfloat2 f = flow(p).segment<2>(0);
    vint2 p1 = p;
    vint2 p2 = (p.cast<float>() + f).cast<int>();
    if (ref_flow(p)[2] > 0 and flow(p).norm() > 0.f
        and i1.has(p + f.cast<int>()) and
        (ref_flow(p).segment<2>(0) - f).norm() > 10)
    {
      display(p1) = vuchar3(0,0, 255);
    }
    else if (ref_flow(p)[2] > 0 and flow(p).norm() > 0.f and
             (ref_flow(p).segment<2>(0) - f).norm() > 3)
    {
      display(p1) = vuchar3(0,255, 255);
    }
    else if (ref_flow(p)[2] > 0 and flow(p).norm() > 0.f and
             (ref_flow(p).segment<2>(0) - f).norm() > 1)
    {
      display(p1) = vuchar3(255,255, 0);
    }
    else if (ref_flow(p)[2] > 0 and flow(p).norm() > 0.f and
             (ref_flow(p).segment<2>(0) - f).norm() <= 1)
    {
      display(p1) = vuchar3(0,255, 0);
    }

  }
  cv::imwrite(path, to_opencv(display));
}

int main(int argc, const char* argv[])
{

  if (argc != 5)
  {
    std::cerr << "Usage: " << argv[0] << " kitti_root n_images config_file result_file" << std::endl;
    return 1;
  }

  auto params = gpof::read_parameters(argv[3],
                                      _nscales = 1,
                                      _winsize = 9,
                                      _propagation = 2,
                                      _min_scale = 0,
                                      _patchsize = 5,
                                      _detector_th = 10,
                                      _block_size = 10);

  int nframes = atoi(argv[2]);

  stats runtime_stats;
  stats error_stats;
  stats nkeypoints_stats;
  
  kitti::foreach_training_pair
    (argv[1], nframes,
     [&] (const image2d<vuchar3>& frame1, const image2d<vuchar3>& frame2,
          const image2d<vfloat3>& ref_flow)
     {
       image2d<uint8_t> i1_gl = rgb_to_graylevel<uint8_t>(frame1);
       image2d<uint8_t> i2_gl = rgb_to_graylevel<uint8_t>(frame2);

       i1_gl = clone(i1_gl, _border = params.winsize, _aligned = 128);
       i2_gl = clone(i2_gl, _border = params.winsize, _aligned = 128);
       
       // Detect keypoints
       auto keypoints = fast9(i1_gl, params.detector_th,
                              _blockwise, _block_size = params.block_size);
       
       image2d<vfloat3> flow(frame1.domain());
       fill(flow, vfloat3(0,0,0));

       // Compute optical flow
       timer t;
       t.start();


       // std::vector<vint2> next_kps;
       // video_extruder_optical_flow2
       //   (i1_gl, i2_gl,
       //    keypoints, next_kps,
       //    params.nscales, params.winsize, params.propagation);

       // for (int i = 0; i < next_kps.size(); i++)
       // {
       //  if (next_kps[i][0] >= 0)
       //  {
       //    vint2 p1 = keypoints[i];
       //    vint2 p2 = next_kps[i];
          
       //    flow(p1).segment<2>(0) = (p2 - p1).cast<float>();
       //    flow(p1)[2] = 1.f;         
       //  }
       // }

       semi_dense_optical_flow
         (keypoints,
          [&] (int i, vint2 pos, int distance) {
             flow(keypoints[i])[2] = 1.f;
             flow(keypoints[i]).segment<2>(0) = vint2(pos - keypoints[i]).cast<float>();
           },
          i1_gl, i2_gl,
          _winsize = params.winsize,
          _propagation = params.propagation,
          _nscales = params.nscales,
          _patchsize = params.patchsize,
          _min_scale = params.min_scale);
       t.end();

       auto flow_errors = kitti::flow_error_stats(flow, ref_flow);
       // display_flow_errors(flow, ref_flow, frame1, "errors.ppm");
       // display_flow(flow, "flow.ppm");
       runtime_stats.take(t.us());
       error_stats.take(flow_errors.n3);
       nkeypoints_stats.take(keypoints.size());
     });

  
  gpof::write_results(argv[4],
                      _runtime = runtime_stats.avg(),
                      _errors = error_stats.avg(),
                      _nkeypoints = nkeypoints_stats.avg());
}
