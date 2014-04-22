#ifndef VPP_IMAGE2d_HH__
# define VPP_IMAGE2d_HH__

namespace vpp
{

  template <typename V>
  class image2d : public imageNd<V, 2>
  {
  public:
    typedef imageNd<V, 2> super;

    // Allocate the image.
    image2d(int nrows, int ncols)
      : super(nrows, ncols)
    {
    }

    // Move constructor.
    image2d(image2d<V, 2>&& other);

    // Accessor
    V& operator()(const vintX<2>& p);
    const V& operator()(const vintX<2>& p) const;

    // Buffer.
    V* data();
    const V* data() const;

  };

  template <typename V, 2>
  using shared_image2d = shared_ptr<image2d<V, 2> >&;

  template <typename V, unsigned 2>
  image2d<V, 2> clone(image2d& img);
};

#endif
