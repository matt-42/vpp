#ifndef VPP_CORE_COLORSPACE_CONVERSIONS_HH__
# define VPP_CORE_COLORSPACE_CONVERSIONS_HH__

# include <vpp/core/pixel_wise.hh>

namespace vpp
{

  template <typename T, typename U>
  void rgb_to_graylevel(const vector<U, 3>& i, vector<T, 1>& o)
  {
    o[0] = (i[0] + i[1] + i[2]) / 3;
  }

  template <typename T, typename U, unsigned N>
  imageNd<T, N> rgb_to_graylevel(const imageNd<vector<U, 3>, N>& in)
  {
    typedef T out_type;
    typedef vector<U, 3> in_type;
    imageNd<out_type, N> out(in.domain(), _border = in.border());
    pixel_wise(in, out) | [] (const in_type& i, out_type& o)
    {
      o = T((i[0] + i[1] + i[2]) / 3);
    };
    return out;
  }

  template <typename T, typename U, unsigned N>
  imageNd<T, N> graylevel_to_rgb(const imageNd<U, N>& in)
  {
    typedef T out_type;
    typedef U in_type;
    imageNd<out_type, N> out(in.domain(), _border = in.border());
    pixel_wise(in, out) | [] (const in_type& i, out_type& o)
    {
      o = out_type(i, i, i);
    };

    return out;
  }
  
};

#endif
