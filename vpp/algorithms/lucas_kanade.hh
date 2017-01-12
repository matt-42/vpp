#pragma once

#include <vpp/core/image2d.hh>

namespace vpp
{

  // lucas_kanade(i1, i2,
  //              _keypoints = K1,
  //              _niterations = lk_iterations,
  //              _winsize = winsize,
  //              _nscales = 3,
  //              _prediction = flow,
  //              _flow = [&] (int i, vfloat2 f) {
  //                auto d2 = extract_patch(K1[i].template cast<float>() + f, i2, winsize);
  //                auto dist = sparse_of_iternals::sad_distance(D1[i], d2);
  //                options.flow(K1[i], f, dist);
  //              }
  //              );

  template <typename... OPTS>
  void lucas_kanade(const image2d<vuchar3>& i1,
		    const image2d<vuchar3>& i2,
		    OPTS... opts);

}

#include <vpp/algorithms/lucas_kanade/lucas_kanade.hpp>
