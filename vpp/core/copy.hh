#ifndef VPP_COPY_HH__
# define VPP_COPY_HH__

# include <vpp/core/parallel_for.hh>

namespace vpp
{

  template <typename I, typename J>
  void copy(I& src, J& dst)
  {
    pixel_wise(src, dst) << [=] (auto& in, auto& out) { out = in; };
  }

};

#endif
