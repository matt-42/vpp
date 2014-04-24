#ifndef VPP_IMAGE3D_HH__
# define VPP_IMAGE3D_HH__

# include <memory>
# include <vpp/imageNd.hh>
# include <vpp/boxNd.hh>

namespace vpp
{

  template <typename V>
  class image3d : public imageNd<V, 3>
  {
  public:
    typedef imageNd<V, 3> super;

    using super::super;

    image3d(int nslices, int nrows, int ncols, int border = 0)
      : super(make_box3d(nslices, nrows, ncols), border)
    {}

    int nslices() const { return super::domain().size(0); }
    int nrows() const { return super::domain().size(1); }
    int ncols() const { return super::domain().size(2); }

  };

  template <typename V>
  using shared_image3d = std::shared_ptr<image3d<V> >;

};

#endif
