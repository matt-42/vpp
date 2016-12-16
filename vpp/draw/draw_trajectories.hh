#pragma once

#include <vpp/core/image2d.hh>
#include <vpp/draw/draw.hh>
#include <vpp/core/keypoint_trajectory.hh>
#include <iod/sio_utils.hh>

namespace vpp
{

  template <typename... OPTS>
  void draw_trajectories(image2d<vuchar3>& out, std::vector<keypoint_trajectory>& trs,
                         int max_trajectory_len,
                         OPTS... opts)
  {
    auto options = iod::D(opts...);

    auto trajectory_color = options.get(_trajectory_color, [] (int i) {
        return vuchar3(255, 255, 0); });
    
    if (max_trajectory_len == -1)
      max_trajectory_len = INT_MAX;

    std::vector<int> sorted_idx;
    for (auto& t : trs) sorted_idx.push_back(sorted_idx.size());

    std::sort(sorted_idx.begin(), sorted_idx.end(), [&] (auto i, auto j) { return trs[i].start_frame() > trs[j].start_frame(); });

#pragma omp parallel for
    for (int ti = 0; ti < trs.size(); ti++)
    {
      keypoint_trajectory& t = trs[sorted_idx[ti]];
      if (!t.alive() or t.size() == 0) continue;

      vint2 p1 = t.position_at_frame(t.end_frame()).template cast<int>();
      vint2 p2 = t.position_at_frame(t.end_frame() - std::min(10, int(t.size()) - 1)).template cast<int>();

      vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p2[0] - p1[0], p2[1] - p1[1])) * 180.f / M_PI, 1.f, 1.f);

      int last_frame_id = std::max(t.end_frame() - max_trajectory_len, t.start_frame());
      if ((p1 - t.position_at_frame(last_frame_id).template cast<int>()).norm() < 4)
        continue;

      for (int i = t.end_frame(); i >= std::max(t.end_frame() - max_trajectory_len, t.start_frame()) + 1; i--)
      {
        vint2 p1 = t.position_at_frame(i).template cast<int>();
        vint2 p2 = t.position_at_frame(i - 1).template cast<int>();
        vuchar4 color;
        color.segment<3>(0) = pt_color;
        color[3] = 0.4f*(255.f - 255.f * (t.end_frame() - i) / max_trajectory_len);

        color = vuchar4(0,255,0,255);
        draw::line2d(out, p1, p2,
                     color
                     );
      }

      //draw::c9(out, t.position().template cast<int>(), pt_color);
      draw::c9(out, t.position().template cast<int>(), vuchar3(0,0,255));

    }
  }

}
