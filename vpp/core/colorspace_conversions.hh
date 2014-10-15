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
  imageNd<vector<T, 1>, N> rgb_to_graylevel(const imageNd<vector<U, 3>, N>& in)
  {
    typedef vector<T, 1> out_type;
    typedef vector<U, 3> in_type;
    imageNd<out_type, N> out(in.domain(), in.border());
    pixel_wise(in, out) << [] (const in_type& i, out_type& o)
    {
      o[0] = (i[0] + i[1] + i[2]) / 3;
    };
    return out;
  }


  template <typename T, typename U, unsigned N>
  imageNd<vector<T, 3>, N> graylevel_to_rgb(const imageNd<vector<U, 1>, N>& in)
  {
    typedef vector<T, 3> out_type;
    typedef vector<U, 1> in_type;
    imageNd<out_type, N> out(in.domain(), in.border());
    pixel_wise(in, out) << [] (const in_type& i, out_type& o)
    {
      o = out_type{i[0], i[0], i[0]};
    };

    return out;
  }

};

#endif
