#ifndef VPP_CLONE_HH__
# define VPP_CLONE_HH__

# include <vpp/core/copy.hh>

namespace vpp
{

  template <typename I, typename... O>
  I clone(I img, const O&... options)
  {
    I n(img.domain(), options...);
    copy(img, n);
    return n;
  }

};

#endif
