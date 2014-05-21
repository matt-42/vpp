#ifndef VPP_IMAGE3D_HH__
# define VPP_IMAGE3D_HH__

# include <vpp/core/imageNd.hh>

namespace vpp
{
  template <typename V>
  using image3d = imageNd<V, 3>;

};

#endif
