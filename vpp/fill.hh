#ifndef VPP_NEIGHBORHOOD_HH__
# define VPP_NEIGHBORHOOD_HH__

# include <vpp/boxNd.hh>
# include <vpp/vector.hh>
# include <vpp/parallel_for.hh>

namespace vpp
{

  template <typename V>
  void fill(imageNd<V>& img, V value)
  {
    pixel_wise(img) << [=] (auto pix) { pix = value; };
  }

};

#endif
