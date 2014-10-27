#ifndef VPP_COPY_HH__
# define VPP_COPY_HH__

namespace vpp
{

  template <typename P, typename... PS,
            typename X = typename std::remove_reference_t<P>::row_iterator>
  auto pixel_wise(P&& p, PS&&... params);

  template <typename I, typename J>
  void copy(const I& src, J& dst)
  {
    pixel_wise(src, dst) | [] (const auto& in, auto& out) { out = in; };
  }

};

#endif
