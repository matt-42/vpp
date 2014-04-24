#ifndef VPP_IMAGE2D_HH__
# define VPP_IMAGE2D_HH__

# include <memory>
# include <vpp/imageNd.hh>
# include <vpp/boxNd.hh>


namespace vpp
{

  template <typename V>
  class image2d : public imageNd<V, 2>
  {
  public:
    typedef imageNd<V, 2> super;

    using super::super;

    image2d(int nrows, int ncols, int border = 0)
      : super(make_box2d(nrows, ncols), border)
    {}

    int nrows() const { return super::domain().size(0); }
    int ncols() const { return super::domain().size(1); }

  };

  template <typename V>
  using shared_image2d = std::shared_ptr<image2d<V> >;

};

#endif
