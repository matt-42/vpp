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


  template <typename T, typename U>
  void rgb_to_graylevel(const vector<U, 3>& i, T& o)
  {
    o = (i[0] + i[1] + i[2]) / 3;
  }
  
  template <typename T, typename U, unsigned N>
  imageNd<T, N> rgb_to_graylevel(const imageNd<vector<U, 3>, N>& in)
  {
    typedef T out_type;
    typedef vector<U, 3> in_type;
    imageNd<out_type, N> out(in.domain(), _border = in.border(), _aligned = in.alignment());
    pixel_wise(in.domain_with_border(), in, out) | [] (vint2, const in_type& i, out_type& o)
    {
      rgb_to_graylevel(i, o);
    };
    return out;
  }
  
  template <typename T, typename U, unsigned N>
  imageNd<T, N> rgb_to_graylevel(const imageNd<vector<U, 4>, N>& in)
  {
    typedef T out_type;
    typedef vector<U, 4> in_type;
    imageNd<out_type, N> out(in.domain(), _border = in.border(), _aligned = in.alignment());
    pixel_wise(in.domain_with_border(), in, out) | [] (vint2, const in_type& i, out_type& o)
    {
      vector<U, 3> tmp = i.template segment<3>(0);
      rgb_to_graylevel(tmp, o);
    };
    return out;
  }
  
  template <typename T, typename U, unsigned N>
  imageNd<T, N> graylevel_to_rgb(const imageNd<U, N>& in)
  {
    typedef T out_type;
    typedef U in_type;
    imageNd<out_type, N> out(in.domain(), _border = in.border(), _aligned = in.alignment());
    pixel_wise(in.domain_with_border(), in, out) | [] (vint2, const in_type& i, out_type& o)
    {
      o = out_type(i, i, i);
    };

    return out;
  }

  inline vuchar3 hsv_to_rgb(int h, float s, float v)
  {
    float c = s * v;
    float h2 = h / 60.f;
    float x = c * (1 - fabs(fmod(h2, 2) - 1));

    unsigned char C = c * 255;
    unsigned char X = x * 255;
    if (h2 < 1)
      return vuchar3(C, X, 0);
    else if (h2 < 2)
      return vuchar3(X, C, 0);
    else if (h2 < 3)
      return vuchar3(0, C, X);
    else if (h2 < 4)
      return vuchar3(0, X, C);
    else if (h2 < 5)
      return vuchar3(X, 0, C);
    else if (h2 < 6)
      return vuchar3(C, 0, X);
    else
      return vuchar3(0,0,0);
  }
  
};

#endif
