#pragma once

#include <iod/array_view.hh>
#include <vpp/core/keypoint_trajectory.hh>
#include <vpp/core/keypoint_container.hh>
#include <vpp/core/symbols.hh>
#include <vpp/algorithms/video_extruder/optical_flow.hh>
#include <vpp/algorithms/fast_detector/fast.hh>

namespace vpp
{
  struct video_extruder_ctx
  {
    video_extruder_ctx(box2d domain) : keypoints(domain) {}

    keypoint_container<keypoint<int>, int> keypoints;
    std::vector<keypoint_trajectory> trajectories;
  };


  // Init the video extruder.
  video_extruder_ctx video_extruder_init(box2d domain)
  {
    video_extruder_ctx res(domain);

    // Setup parameters ?

    return res;
  }

  // Update the video extruder with a new frame.
  template <typename... OPTS>
  void video_extruder_update(video_extruder_ctx& ctx,
                             const image2d<unsigned char>& frame1,
                             const image2d<unsigned char>& frame2,
                             //float precision_runtime_balance = 0,
                             OPTS... options)
  {
    // Options.
    auto opts = D(options...);
    int detector_th = opts.get(_detector_th, 10);
    int keypoint_spacing = opts.get(_keypoint_spacing, 10);
    int detector_period = opts.get(_detector_period, 5);

    // Optical flow vectors.
    ctx.keypoints.prepare_matching();
    semi_dense_optical_flow(iod::array_view(ctx.keypoints.size(),
                                       [&] (int i) { return ctx.keypoints[i].position; }),
                            [&] (int i, vint2 pos, int distance) { ctx.keypoints.move(i, pos); },
                            frame1,
                            frame2);

    // Filtering.
    // Merging.
    {
      image2d<int> idx(frame2.domain().nrows() / keypoint_spacing,
                       frame2.domain().ncols() / keypoint_spacing);

      fill(idx, -1);
      for (int i = 0; i < ctx.keypoints.size(); i++)
      {
        vint2 pos = ctx.keypoints[i].position / keypoint_spacing;
        assert(idx.has(pos));
        if (idx(pos) >= 0)
        {
          auto& other = ctx.keypoints[idx(pos)];
          if (other.age < ctx.keypoints[i].age)
          {
            ctx.keypoints.remove(idx(pos));
            idx(pos) = i;
          }
          else
            ctx.keypoints.remove(i);
        }
        else
          idx(pos) = i;
      }
    }

    // Detect new keypoints.
    {
      image2d<int> mask(frame2.domain().nrows(),
                        frame2.domain().ncols(),
                        _border = keypoint_spacing);

      fill_with_border(mask, 0);
      for (int i = 0; i < ctx.keypoints.size(); i++)
      {
        int r = ctx.keypoints[i].position[0];
        int c = ctx.keypoints[i].position[1];
        for (int dr = -keypoint_spacing; dr < -keypoint_spacing; dr++)
          for (int dc = -keypoint_spacing; dc < -keypoint_spacing; dc++)
            mask[r + dr][c + dc] = 1;
      }

      auto kps = fast9(frame2, detector_th, _blockwise,
                       _block_size = keypoint_spacing,
                       _mask = mask);
      for (auto kp : kps)
        ctx.keypoints.add(keypoint<int>(kp));
      
      // Trajectories update.
      ctx.keypoints.sync_attributes(ctx.trajectories);
    }
  }
  
}
