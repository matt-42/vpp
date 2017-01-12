#pragma once

#include <vpp/vpp.hh>
#include <vpp/algorithms/fast_detector/fast.hh>


namespace vpp
{

  namespace sparse_of_iternals
  {

    template <typename F>
    auto sad_distance(const F& i1, const F& i2, int size, int th = INT_MAX)
    {
      const int RS = 128;
      int err = 0;
      for (int r = 0; r < (size)/RS and err <= th; r++)
      {
#pragma omp simd
        for (int i = 0; i < RS; i++)
          err += std::abs((i1[r * RS + i]) - (i2[r * RS + i]));
      }

      if (err <= th)
      {
#pragma omp simd
        for (int i = (((size)/RS)*RS); i < size; i++)
          err += std::abs((i1[i]) - (i2[i]));
      }

      return err;
    }
  }

#if 0 // FIXME: Reactivate when it compiles.
  
  template <typename K, typename F, typename... OPTS>
  inline void
  sparse_optical_flow(const image2d<unsigned char>& i1,
                      const image2d<unsigned char>& i2,
                      OPTS... opts)
  {
    auto options = D(opts...);
    const int block_size = options.get(_block_size, 5);
    const int detector_th = options.get(_detector_th, 5);
    constexpr int is_subpixelic = options.get(_subpixelic, 5);
    const int lk_iterations = options.get(_lk_iterations, 5);
    const int winsize = options.get(_winsize, 11);

    auto flow_callback = options.flow;
    std::vector<vint2> K1 = fast9(i1,
                                  detector_th,
                                  _blockwise,
                                  _block_size = block_size);

    std::vector<vint2> K2 = fast9(i2,
                                  detector_th,
                                  _blockwise,
                                  _block_size = block_size);

    auto D1
      = extract_patches(K1, i1, winsize);
    auto D2
      = extract_patches(K2, i2, winsize);

    if (is_subpixelic)
    {
      match_descritors
        (_query = D1,
         _train = D2,
         _distance = sparse_of_iternals::sad_distance,
         _match = [&] (int q, int t, int d) { options.flow(K1[q], K2[t] - K1[q], d); },

         _index1d(_approximation = 2),
         _local_search(_query_positions = K1,
                       _train_positions = K2,
                       _radius = 100)
         );
    }
    else
    {
      std::vector<vint2> flow(K1.size());
      fill(flow, vint2(0,0));

      match_descritors
        (_query = D1,
         _train = D2,
         _distance = sparse_optical_flow::sad_distance,
         _match = [&] (int q, int t, int d) { flow[q] = vint2(K2[t] - K1[q]); },

         _index1d(_approximation = 2),
         _local_search(_query_positions = K1,
                       _train_positions = K2,
                       _radius = 100)
         );

      lucas_kanade(i1, i2,
                   _keypoints = K1,
                   _niterations = lk_iterations,
                   _winsize = winsize,
                   _prediction = flow,
                   _flow = [&] (int i, vfloat2 f) {
                     auto d2 = extract_patch(K1[i].template cast<float>() + f, i2, winsize);
                     auto dist = sparse_of_iternals::sad_distance(D1[i], d2);
                     options.flow(K1[i], f, dist);
                   }
                   );
    }
  }

  #endif
}
