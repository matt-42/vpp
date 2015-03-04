#ifndef VPP_CORE_DIGE_HH_
# define VPP_CORE_DIGE_HH_

# include <dige/image.h>
# include <vpp/core/image2d.hh>

namespace dg
{

  inline image<trait::format::bgr, unsigned char>
  adapt(const vpp::image2d<vpp::vuchar3>& i)
  {
    return image<trait::format::bgr, unsigned char>
      (i.ncols() + 2*i.border(), i.nrows() + 2*i.border(), (unsigned char*)&i(0,0));
  }

  inline image<trait::format::luminance, unsigned char>
  adapt(const vpp::image2d<vpp::vuchar1>& i)
  {
    return image<trait::format::luminance, unsigned char>
      (i.ncols() + 2*i.border(), i.nrows() + 2*i.border(), (unsigned char*)i.data());
  }


  inline image<trait::format::luminance, unsigned char>
  adapt(const vpp::image2d<unsigned char>& i)
  {
    return image<trait::format::luminance, unsigned char>
      (i.ncols() + 2*i.border(), i.nrows() + 2*i.border(), (unsigned char*)i.data());
  }
  
};

#endif
