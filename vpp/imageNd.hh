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

    imageNd(const std::initializer_list<int>& dims, border b = 0);
    imageNd(const std::vector<int>& dims, border b = 0);

    imageNd(int ncols, border b = 0); // 1D.
    imageNd(int nrows, int ncols, border b = 0); // 2D.
    imageNd(int nslices, int nrows, int ncols, border b = 0); // 3D.

    imageNd(const boxNd<N>& domain, border b = 0);

    imageNd(std::vector<int> dims, border b, V* data, int pitch, bool own_data = false);

    // Copy constructor. Share the data.
    imageNd(const imageNd<V, N>& other);

    // Move constructor.
    imageNd(const imageNd<V, N>&& other);

    // Destructor.
    ~imageNd();

    inline int nslices() const { static_assert(N >= 3, "nslices require dimension >= 3."); return domain().size(N-3); }
    inline int nrows() const { static_assert(N >= 2, "nrows require dimension >= 2."); return domain().size(N-2); }
    inline int ncols() const { static_assert(N >= 1, "ncols require dimension >= 1."); return domain().size(N-1); }

    // Assigment.
    imageNd<V, N>& operator=(const imageNd<V, N>& other);
    imageNd<V, N>& operator=(const imageNd<V, N>&& other);

    // Access to values.
    inline V& operator()(const vint<N>& p);
    inline const V& operator()(const vint<N>& p) const;

    template <typename... Tail>
    inline const V& operator()(int c, Tail... cs) const
    {
      static_assert(1 + sizeof...(cs) == N, "Wrong dimension of coordinates passed to imageNd::operator().");
      return operator()(coord_type(c, cs...));
    }

    template <typename... Tail>
    inline V& operator()(int c, Tail... cs)
    {
      static_assert(1 + sizeof...(cs) == N, "Wrong dimension of coordinates passed to imageNd::operator().");
      return operator()(coord_type(c, cs...));
    }


    inline V* address_of(const vint<N>& p);
    inline const V* address_of(const vint<N>& p) const;

    inline int offset_of(const vint<N>& p) const;

    inline int coords_to_index(const vint<N>& p) const;

    inline int coords_to_offset(const vint<N>& p) const;

    inline bool has(coord_type& p) const { return ptr_->domain_.has(p); }
    inline bool has(const V* p) const { return p >= ptr_->data_ and p < ptr_->data_end_; }
    // Access to raw buffer.
    inline V* data() { return ptr_->data_; }
    inline const V* data() const { return ptr_->data_; }
    inline const V* data_end() const { return ptr_->data_end_; }

    inline iterator begin() { return iterator(*ptr_->domain_.begin(), *this); }
    inline iterator end() { return iterator(*ptr_->domain_.end(), *this); }

    inline int pitch() const { return ptr_->pitch_; }
    inline int border() const { return ptr_->border_; }

    // Domain
    inline const boxNd<N>& domain() const { return ptr_->domain_; }
    inline boxNd<N> domain_with_border() const { return ptr_->domain_ + vpp::border(ptr_->border_); }

    inline void set_external_refcount (int* refcount) { ptr_->external_refcount_ = refcount; }

  protected:
    void allocate(const std::vector<int>& dims, vpp::border b);

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
