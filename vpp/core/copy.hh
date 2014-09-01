#ifndef VPP_COPY_HH__
# define VPP_COPY_HH__

# include <vpp/core/parallel_for.hh>

namespace vpp
{

  template <typename I, typename J>
  void copy(const I& src, J& dst)
  {
    pixel_wise(src, dst) << [] (const auto& in, auto& out) { out = in; };
  }

};

#endif
