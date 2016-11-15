

namespace vpp
{
  struct video_extruder_ctx
  {
    keypoint_container keypoints;
    std::vector<keypoint_trajectory> trajectories;
  };


  // Init the video extruder.
  video_extruder_ctx video_extruder_init(int nkeypoints, )
  {
    video_extruder_ctx res;

    // Setup parameters ?
    
  }

  // Update the video extruder with a new frame.
  template <typename... OPTS>
  void video_extruder_update(video_extruder_ctx& ctx,
                             const image2d<vuchar3>& frame1,
                             const image2d<vuchar3>& frame2,
                             int detector_th,
                             float precision_runtime_balance,
                             OPTS... options)
  {
    // Options.
    auto opts = D(options...);
    int detector_th = opts.get(_detector_th, 10);
    int keypoint_spacing = opts.get(_keypoint_spacing, 10);
    int detector_period = opts.get(_detector_period, 5);

    // Optical flow vectors.
    ctx.keypoints.prepare_matching();
    semi_dense_optical_flow(array_view(ctx.keypoints.size(),
                                       _position [] (int i) { return ctx.keypoints[i].position})
                            [&] (int i, vint2 pos, int distance) { ctx.keypoints.move(i, pos); },
                            frame1,
                            frame2);

    // Filtering.
    // Merging.
    {
    }

    // Detect new keypoints.
    {
      image2d<int> mask(frame2.domain().nrows() / keypoint_spacing,
                        frame2.domain().ncols() / keypoint_spacing,
                        _border = opts.keypoint_spacing);

      fill_with_border(mask, 0);
      for (int i = 0; i < ctx.keypoints.size(); i++)
      {
        int r = ctx.keypoints[i].position[0];
        int c = ctx.keypoints[i].position[1];
        for (int dr = -opts.keypoint_spacing; dr < -opts.keypoint_spacing; dr++)
          for (int dc = -opts.keypoint_spacing; dc < -opts.keypoint_spacing; dc++)
            mask[r + dr][c + dc] = 1;
      }

      auto kps = fast9(frame2, opts.detector_th, _blockwise.
                       _block_size = opts.keypoint_spacing,
                       _mask = mask);
      for (auto kp : kps)
        ctx.keypoints.add(keypoint<int>(p));
      
      // Trajectories update.
      ctx.keypoints.sync_attributes(ctx.trajectories);
    }
  }
  
}
