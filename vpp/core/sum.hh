#ifndef VPP_SUM_HH__
# define VPP_SUM_HH__

# include <vpp/core/imageNd.hh>
# include <vpp/core/pixel_wise.hh>
# include <vpp/core/zero.hh>

namespace vpp
{

  template <typename V, unsigned N>
  plus_promotion<V> sum(const imageNd<V, N>& img)
  {
    typedef plus_promotion<V> S;
    S res = zero<S>();
    pixel_wise(img)(_no_threads) | [&res] (auto v) { res += v; };
    return res;

  }
};

#endif
