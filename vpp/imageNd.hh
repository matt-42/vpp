#ifndef VPP_IMAGENd_HH__
# define VPP_IMAGENd_HH__

namespace vpp
{

  template <typename V, unsigned N>
  class imageNd
  {
  public:

    // Allocate the image.
    imageNd(int dims[], int border = 0);

    imageNd(const domainNd<N>& domain_, int border = 0);

    // Destructor.
    ~imageNd();

    // Move constructor.
    imageNd(imageNd<V, N>&& other) = default;

    // Assigment.
    imageNd<V, N>& operator=(imageNd<V, N>& other) = default;

    // Accessor
    V& operator()(const vintX<N>& p);
    const V& operator()(const vintX<N>& p) const;

    int coords_to_index() const;

    // Buffer.
    V* data();
    const V* data() const;

  private:

    void allocate(int dims[N], int border);
    V* buffer_, begin_;
    domainNd<N> domain_;
    int border_;
  };

  template <typename V, N>
  using shared_imageNd = shared_ptr<imageNd<V, N> >&;

  template <typename V, unsigned N>
  imageNd<V, N> clone(imageNd& img);

};

#endif
