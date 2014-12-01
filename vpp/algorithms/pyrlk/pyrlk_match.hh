#ifndef VPP_ALGORITHMS_PYRLK_HH_
# define VPP_ALGORITHMS_PYRLK_HH_

# include <vpp/core/pyramid.hh>
# include <vpp/core/keypoint_container.hh>
# include <vpp/algorithms/pyrlk/lk.hh>
# include <vpp/core/vector.hh>

namespace vpp
{

  typedef keypoint_container<keypoint<float>, int> pyrlk_keypoint_container;

  template <typename M, typename V, typename U, typename C>
  void pyrlk_match(const pyramid2d<vector<V, 1>>& pyramid_prev,
                   const pyramid2d<vector<U, 2>>& pyramid_prev_grad,
                   const pyramid2d<vector<V, 1>>& pyramid_next,
                   C& keypoints,
                   M matcher,
                   float min_ev, float max_err,
                   float max_iteration, float convergence_delta, int min_scale = 0)
  {
    keypoints.prepare_matching();
    #pragma omp parallel for
    for(int i = 0; i < keypoints.size(); i++)
    {
      auto& kp = keypoints[i];
      if (kp.alive())
      {
        vfloat2 tr = vfloat2{0.f,0.f};
        float dist = 0.f;
        for(int S = pyramid_prev.size() - 1; S >= min_scale; S--)
        {
          tr *= pyramid_prev.factor();
          auto match = matcher(kp.position / std::pow(2, S), tr, pyramid_prev[S], pyramid_next[S], pyramid_prev_grad[S], min_ev, max_iteration, convergence_delta);

          if (match.second < max_err)
          {
            tr = match.first;
          }
          dist = match.second;
          // else
          // {
          //   keypoints.remove(i);
          //   goto nextpoint;
          // }
        }

        if (dist > max_err || !pyramid_prev[0].domain().has(cast<vint2>(kp.position + tr)))
        {
        remove_keypoint:
          keypoints.remove(i);
        }
        else
          keypoints.move(i, kp.position + tr);
      }
    nextpoint:
      continue;
    }
  }


  template <typename M, typename V, typename U, typename C>
  void pyrlk_match2(const pyramid2d<vector<V, 1>>& pyramid_prev,
                   const pyramid2d<vector<U, 2>>& pyramid_prev_grad,
                   const pyramid2d<vector<V, 1>>& pyramid_next,
                   C& keypoints,
                   std::vector<float>* errors,
                   M matcher,
                   float min_ev, float max_err,
                   float max_iteration, float convergence_delta)
  {
    keypoints.prepare_matching();
    std::vector<vfloat2> trs(keypoints.size(), vfloat2{0.f, 0.f});

    for(int S = pyramid_prev.size() - 1; S >= 0; S--)
    {
      //#pragma omp parallel for
      for(int i = 0; i < keypoints.size(); i++)
      {
        auto& kp = keypoints[i];
        if (kp.alive())
        {
          vfloat2 tr = trs[i];
          float dist = 0.f;
          {
            tr *= pyramid_prev.factor();
            auto match = matcher(kp.position / std::pow(2, S), tr, pyramid_prev[S], pyramid_next[S], pyramid_prev_grad[S], min_ev, max_iteration, convergence_delta);

            if (match.second < max_err)
            {
              tr = match.first;
            }
            dist = match.second;
          }

          if (S == 0)
          {
            if (errors) errors->operator[](i) = dist;
            if (dist > max_err || !pyramid_prev[0].domain().has(cast<vint2>(kp.position + tr)))
            {
            remove_keypoint:
              keypoints.remove(i);
            }
            else
              keypoints.move(i, kp.position + tr);
          }
          else trs[i] = tr;
        }
      nextpoint:
        continue;
      }
    }
  }

}

#endif
