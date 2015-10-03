#ifndef VPP_CLONE_HH__
# define VPP_CLONE_HH__

# include <vpp/core/copy.hh>

namespace vpp
{

  template <typename I, typename... O>
  I clone(I img, const O&... options)
  {
    auto o = iod::D(options...);
    int border = o.has(_border) ? o.get(_border, 0) : img.border();
    int aligned = o.has(_aligned) ? o.get(_aligned, 0) : img.alignment();
    assert(aligned != 0);
    I n(img.domain(), _border = border, _aligned = aligned);
    copy_with_border(img, n);
    return n;
  }

};

#endif
