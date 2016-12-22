#pragma once

#include <iod/array_view.hh>
#include <vpp/core/keypoint_trajectory.hh>
#include <vpp/core/keypoint_container.hh>
#include <vpp/core/symbols.hh>
#include <vpp/algorithms/symbols.hh>
#include <vpp/algorithms/optical_flow.hh>
#include <vpp/algorithms/fast_detector/fast.hh>

namespace vpp
{

  // Init the video extruder.
  video_extruder_ctx video_extruder_init(box2d domain)
  {
    video_extruder_ctx res(domain);
    res.frame_id = -1;
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
    ctx.frame_id++;
    
    // Options.
    auto opts = D(options...);

    const int detector_th = opts.get(_detector_th, 10);
    const int keypoint_spacing = opts.get(_keypoint_spacing, 10);
    const int detector_period = opts.get(_detector_period, 5);
    const int max_trajectory_length = opts.get(_max_trajectory_length, 15);
    const int nscales = opts.get(_nscales, 3);
    const int winsize = opts.get(_winsize, 9);
    const int regularisation_niters = opts.get(_propagation, 2);

    // Optical flow vectors.
    ctx.keypoints.prepare_matching();
    semi_dense_optical_flow
      (iod::array_view(ctx.keypoints.size(),
                       [&] (int i) { return ctx.keypoints[i].position; }),
       [&] (int i, vint2 pos, int distance) {
        if (frame1.has(pos))
          ctx.keypoints.move(i, pos);
        else ctx.keypoints.remove(i); },
       frame1, frame2,
       _winsize = winsize,
       _patchsize = 5,
       _propagation = regularisation_niters,
       _nscales = nscales);

    // Filtering.
    // Merge particules that converged to the same pixel.
    {
      image2d<int> idx(frame2.domain().nrows() / keypoint_spacing,
                       frame2.domain().ncols() / keypoint_spacing, _border = 1);

      fill_with_border(idx, -1);
      for (int i = 0; i < ctx.keypoints.size(); i++)
      {
        vint2 pos = ctx.keypoints[i].position / keypoint_spacing;
        //assert(idx.has(pos));
        if (idx(pos) >= 0)
        {
          assert(idx(pos) < ctx.keypoints.size());
          auto other = ctx.keypoints[idx(pos)];
          if (other.age < ctx.keypoints[i].age)
          {
            ctx.keypoints.remove(idx(pos));
            idx(pos) = i;
          }
          if (other.age > ctx.keypoints[i].age)
            ctx.keypoints.remove(i);
        }
        else
          idx(pos) = i;
      }
    }

    // Remove points with very low fast scores.
    {
      for (int i = 0; i < ctx.keypoints.size(); i++)
        if (fast9_score(frame2, detector_th, ctx.keypoints[i].position) < 3)
          ctx.keypoints.remove(i);
    }

    // Detect new keypoints.
    {
      if (!(ctx.frame_id % detector_period))
      {
        image2d<unsigned char> mask(frame2.domain().nrows(),
                          frame2.domain().ncols(),
                          _border = keypoint_spacing);

        fill_with_border(mask, 1);
        for (int i = 0; i < ctx.keypoints.size(); i++)
        {
          int r = ctx.keypoints[i].position[0];
          int c = ctx.keypoints[i].position[1];
          for (int dr = -keypoint_spacing; dr < keypoint_spacing; dr++)
            for (int dc = -keypoint_spacing; dc < keypoint_spacing; dc++)
              mask[r + dr][c + dc] = 0;
        }

        auto kps = fast9(frame2, detector_th, _blockwise,
                         _block_size = keypoint_spacing,
                         _mask = mask);
        for (auto kp : kps)
          ctx.keypoints.add(keypoint<int>(kp));

        ctx.keypoints.compact();
        ctx.keypoints.sync_attributes(ctx.trajectories, keypoint_trajectory(ctx.frame_id));
      }
    }

    // Trajectories update.
    for (int i = 0; i < ctx.keypoints.size(); i++)
    {
      if (ctx.keypoints[i].alive())
      {
        ctx.trajectories[i].move_to(ctx.keypoints[i].position.template cast<float>());
        if (ctx.trajectories[i].size() > max_trajectory_length)
          ctx.trajectories[i].pop_oldest_position();
      }
      else
        ctx.trajectories[i].die();
    }

  }
  
}
