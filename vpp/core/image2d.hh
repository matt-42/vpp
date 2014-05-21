#ifndef VPP_IMAGE2D_HH__
# define VPP_IMAGE2D_HH__

# include <vpp/core/imageNd.hh>

namespace vpp
{

  template <typename V>
  using image2d = imageNd<V, 2>;

}

#endif
