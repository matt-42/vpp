#ifndef VPP_ALGORITHMS_PYRLK_HH_
# define VPP_ALGORITHMS_PYRLK_HH_

# include <vpp/core/pyramid.hh>

namespace vpp
{

  template <typename M, typename V, typename C>
  void pyrlk_match(const pyramid2d<V>& pyramid_prev,
                   const pyramid2d<V>& pyramid_next,
                   C& keypoints,
                   M matcher,
                   float min_ev, float max_err)
  {
    #pragma omp parallel for
    for(int i = 0; i < keypoints.size(); i++)
    {
      auto& kp = keypoints[i];
      vfloat2 tr = vfloat2(0.f,0.f);
      for(int S = 0; S < pyramid_prev.size(); S++)
      {
        tr *= pyramid_prev.factor();
        auto match = M::match(kp.position, tr, pyramid_prev[S], pyramid_next[S], pyramid_prev_grad[S]);
        if (match.second < max_err)
          tr = match.first;
        else
        {
          keypoints.remove(i);
          goto nextpoint;
        }
      }

      kp.position += tr;
      kp.age++;
      keypoints.update_index(i, kp.position.cast<int>());

    nextpoint:
      continue;
    }
  }
}

#endif
