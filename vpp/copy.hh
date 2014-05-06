#ifndef VPP_COPY_HH__
# define VPP_COPY_HH__

# include <vpp/parallel_for.hh>

namespace vpp
{

  template <typename V, unsigned N>
  void copy(imageNd<V, N>& src, imageNd<V, N>& dst)
  {
    pixel_wise(src, dst) << [=] (auto& in, auto& out) { out = in; };
  }

};

#endif
