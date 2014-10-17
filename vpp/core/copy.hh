#ifndef VPP_COPY_HH__
# define VPP_COPY_HH__

namespace vpp
{

  template <typename... PS>
  auto pixel_wise(PS&&... params);

  template <typename I, typename J>
  void copy(const I& src, J& dst)
  {
    pixel_wise(src, dst) | [] (const auto& in, auto& out) { out = in; };
  }

};

#endif
