#ifndef VPP_COPY_HH__
# define VPP_COPY_HH__

namespace vpp
{
  //struct pixel_wise_functor;
  //pixel_wise_functor pixel_wise;

  template <typename I, typename J>
  void copy(const I& src, J& dst)
  {
    pixel_wise(src, dst) | [] (const auto& in, auto& out) { out = in; };
  }

  template <typename I, typename J>
  void copy(const I& src, J&& dst)
  {
    pixel_wise(src, dst) | [] (const auto& in, auto& out) { out = in; };
  }

  template <typename I, typename J>
  void copy_with_border(const I& src, J&& dst)
  {
    assert(src.domain() == dst.domain());
    assert(src.border() <= dst.border());
    pixel_wise(src.domain_with_border(), src, dst) | [] (vint2, const auto& in, auto& out) { out = in; };
  }
  
};

#endif
