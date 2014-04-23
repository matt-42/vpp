#ifndef VPP_IMAGENd_HH__
# define VPP_IMAGENd_HH__

# include <vpp/boxNd.hh>
# include <memory>
namespace vpp
{

  template <typename V, unsigned N>
  class imageNd
  {
  public:

    // Constructors.
    imageNd(int* dims, int border = 0);

    imageNd(const boxNd<N>& domain_, int border = 0);


    // Move constructor.
    imageNd(imageNd<V, N>&& other) = default;

    // Destructor.
    ~imageNd();

    // Assigment.
    imageNd<V, N>& operator=(imageNd<V, N>& other) = default;

    // Access to values.
    V& operator()(const vint<N>& p);
    const V& operator()(const vint<N>& p) const;

    int coords_to_index(const vint<N>& p) const;

    // Access to raw buffer.
    V* data() { return data_; }
    const V* data() const { return data_; }
    V* begin() { return begin_; }
    const V* begin() const { return begin_; }

    // Domain
    const boxNd<N>& domain() { return domain_; }

  private:

    void allocate(int dims[N], int border);
    V* data_;
    V* data_end_;
    V* begin_;
    boxNd<N> domain_;
    int border_;
  };

  template <typename V, unsigned N>
  using shared_imageNd = std::shared_ptr<imageNd<V, N> >&;

  template <typename V, unsigned N>
  imageNd<V, N> clone(imageNd<V, N>& img);

};

# include <vpp/imageNd.hpp>
#endif
