#ifndef VPP_IMAGENd_HH__
# define VPP_IMAGENd_HH__

# include <memory>
# include <vpp/boxNd.hh>
# include <vpp/imageNd_iterator.hh>

namespace vpp
{

  template <typename P>
  void delete_imageNd_data(P* ptr)
  {
    std::cout << "delete" << std::endl;
    std::cout << ptr->external_refcount_ << std::endl;
    if (ptr->external_refcount_ && *(ptr->external_refcount_) > 1)
    {
      (*(ptr->external_refcount_))--;
    }
     else
      delete ptr;
  }

  template <typename V, unsigned N, typename EXTERNAL_REFCOUNT_TYPE = int>
  class imageNd_data
  {
  public:
    V* data_;
    V* data_end_;
    V* begin_;
    boxNd<N> domain_;
    int border_;
    int pitch_;
    bool own_data_;
    EXTERNAL_REFCOUNT_TYPE* external_refcount_;
  };

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

    imageNd(const boxNd<N>& domain, int border = 0);

    imageNd(int* dims, int border, V* data, int pitch, bool own_data = false);

    // Copy constructor. Share the data.
    imageNd(const imageNd<V, N>& other);

    // Move constructor.
    imageNd(const imageNd<V, N>&& other);

    // Destructor.
    ~imageNd();

    int nslices() const { static_assert(N >= 3, "nslices require dimension >= 3."); return domain().size(N-3); }
    int nrows() const { static_assert(N >= 2, "nrows require dimension >= 2."); return domain().size(N-2); }
    int ncols() const { static_assert(N >= 1, "ncols require dimension >= 1."); return domain().size(N-1); }

    // Assigment.
    imageNd<V, N>& operator=(const imageNd<V, N>& other);
    imageNd<V, N>& operator=(const imageNd<V, N>&& other);

    // Access to values.
    V& operator()(const vint<N>& p);
    const V& operator()(const vint<N>& p) const;

    template <typename... Tail>
    const V& operator()(int c, Tail... cs) const
    {
      static_assert(1 + sizeof...(cs) == N, "Wrong dimension of coordinates passed to imageNd::operator().");
      return operator()(coord_type(c, cs...));
    }

    template <typename... Tail>
    V& operator()(int c, Tail... cs)
    {
      static_assert(1 + sizeof...(cs) == N, "Wrong dimension of coordinates passed to imageNd::operator().");
      return operator()(coord_type(c, cs...));
    }


    V* address_of(const vint<N>& p);
    const V* address_of(const vint<N>& p) const;

    int offset_of(const vint<N>& p) const;

    int coords_to_index(const vint<N>& p) const;

    int coords_to_offset(const vint<N>& p) const;

    bool has(coord_type& p) const { return ptr_->domain_.has(p); }
    bool has(const V* p) const { return p >= ptr_->data_ and p < ptr_->data_end_; }
    // Access to raw buffer.
    V* data() { return ptr_->data_; }
    const V* data() const { return ptr_->data_; }
    const V* data_end() const { return ptr_->data_end_; }

    iterator begin() { return iterator(*ptr_->domain_.begin(), *this); }
    iterator end() { return iterator(*ptr_->domain_.end(), *this); }

    int pitch() const { return ptr_->pitch_; }
    int border() const { return ptr_->border_; }

    // Domain
    const boxNd<N>& domain() const { return ptr_->domain_; }

    void set_external_refcount (int* refcount) { ptr_->external_refcount_ = refcount; }

    // Cast to imageNd.
    imageNd<V, N>& up_cast() { return *static_cast<imageNd<V, N>*>(this); }
    const imageNd<V, N>& up_cast() const { return *static_cast<const imageNd<V, N>*>(this); }

  protected:
    void allocate(int dims[N], int border);

    std::shared_ptr<imageNd_data<V, N> > ptr_;
  };

  template <typename V, unsigned N>
  using shared_imageNd = std::shared_ptr<imageNd<V, N> >&;

  template <typename I>
  I clone(I img);

  template <typename V, unsigned N>
  imageNd<V, N> clone_with_border(imageNd<V, N>& img);

};

# include <vpp/imageNd.hpp>
#endif
