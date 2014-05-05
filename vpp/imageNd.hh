#ifndef VPP_IMAGENd_HH__
# define VPP_IMAGENd_HH__

# include <memory>
# include <vpp/boxNd.hh>
# include <vpp/imageNd_iterator.hh>

namespace vpp
{

  template <typename V, unsigned N>
  class imageNd
  {
  public:

    typedef V value_type;
    typedef vint<N> coord_type;
    typedef boxNd<N> domain_type;
    typedef imageNd_iterator<V, N> iterator;
    typedef imageNd_row_iterator<V, N> row_iterator;
    enum { dimension = N };

    // Constructors.
    imageNd();

    imageNd(int* dims, int border = 0);

    imageNd(const boxNd<N>& domain_, int border = 0);

    imageNd(int* dims, int border, V* data, int pitch, bool own_data = false);

    // Copy constructor. Share the data.
    imageNd(const imageNd<V, N>& other);

    // Move constructor.
    imageNd(const imageNd<V, N>&& other);

    // Destructor.
    ~imageNd();

    // Assigment.
    imageNd<V, N>& operator=(const imageNd<V, N>& other);
    imageNd<V, N>& operator=(const imageNd<V, N>&& other);

    // Access to values.
    V& operator()(const vint<N>& p);
    const V& operator()(const vint<N>& p) const;

    V* address_of(const vint<N>& p);
    const V* address_of(const vint<N>& p) const;

    int offset_of(const vint<N>& p) const;

    int coords_to_index(const vint<N>& p) const;

    int coords_to_offset(const vint<N>& p) const;

    bool has(coord_type& p) const { return domain_.has(p); }
    bool has(const V* p) const { return p >= data_ and p < data_end_; }
    // Access to raw buffer.
    V* data() { return data_; }
    const V* data() const { return data_; }

    iterator begin() { return iterator(*domain_.begin(), *this); }
    iterator end() { return iterator(*domain_.end(), *this); }

    int pitch() const { return pitch_; }

    // Domain
    const boxNd<N>& domain() const { return domain_; }

    // Cast to imageNd.
    imageNd<V, N>& up_cast() { return *static_cast<imageNd<V, N>*>(this); }
    const imageNd<V, N>& up_cast() const { return *static_cast<const imageNd<V, N>*>(this); }

  protected:

    void allocate(int dims[N], int border);
    V* data_;
    V* data_end_;
    V* begin_;
    boxNd<N> domain_;
    int border_;
    int pitch_;
    bool own_data_;
  };

  template <typename V, unsigned N>
  using shared_imageNd = std::shared_ptr<imageNd<V, N> >&;

  template <typename V, unsigned N>
  imageNd<V, N> clone(imageNd<V, N>& img);

};

# include <vpp/imageNd.hpp>
#endif
