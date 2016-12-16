#ifndef VPP_IMAGENd_HH__
# define VPP_IMAGENd_HH__

# include <memory>

# include <iod/sio.hh>
# include <iod/sio_utils.hh>

# include <vpp/core/boxNd.hh>
# include <vpp/core/imageNd_iterator.hh>
# include <vpp/core/cast_to_float.hh>

namespace vpp
{

  template <typename V, unsigned N>
  class imageNd_data
  {
  public:
    imageNd_data()
      : data_(nullptr),
        data_end_(nullptr),
        begin_(nullptr),
        border_(0),
        pitch_(0),
        alignment_(0)
      {}

    V* data_;
    std::vector<V*> rows_;
    V** rows_array_start_;
    std::shared_ptr<void> data_sptr_;
    V* data_end_;
    V* begin_;
    boxNd<N> domain_;
    boxNd<N> buffer_domain_;
    int border_;
    int pitch_;
    int alignment_;
  };

  template <typename V, unsigned N>
  class imageNd
  {
  public:

    typedef imageNd<V, N> self;

    typedef V value_type;
    typedef vint<N> coord_type;
    typedef boxNd<N> domain_type;
    typedef imageNd_iterator<V, N, unconstify> iterator;
    typedef imageNd_iterator<V, N, constify> const_iterator;
    typedef imageNd_row_iterator<V, N, unconstify> row_iterator;
    typedef imageNd_row_iterator<V, N, constify> const_row_iterator;
    enum { dimension = N };

    // Constructors.
    imageNd();

    template <typename... O>
    imageNd(const std::initializer_list<int>& dims, const O&... opts_);

    template <typename... O>
    imageNd(const std::vector<int>& dims, const O&... opts_);

    template <typename... O>
    imageNd(int ncols, const O&... opts_); // 1D.

    template <typename... O>
    imageNd(int nrows, int ncols, const O&... opts_); // 2D.

    template <typename... O>
    imageNd(int nslices, int nrows, int ncols, const O&... opts_); // 3D.

    template <typename... O>
    imageNd(const boxNd<N>& domain, const O&... opts_);

    // Copy constructor. Share the data.
    imageNd(const imageNd<V, N>& other);

    // Move constructor.
    imageNd(imageNd<V, N>&& other);

    // Destructor.
    ~imageNd();

    inline const int& nslices() const { static_assert(N >= 3, "nslices require dimension >= 3."); return domain().size(N-3); }
    inline const int& nrows() const { static_assert(N >= 2, "nrows require dimension >= 2."); return domain().size(N-2); }
    inline const int& ncols() const { static_assert(N >= 1, "ncols require dimension >= 1."); return domain().size(N-1); }

    // Assigment.
    imageNd<V, N>& operator=(const imageNd<V, N>& other);
    imageNd<V, N>& operator=(imageNd<V, N>&& other);

    // Access to values.
    inline V& operator()(const vint<N>& p);
    inline const V& operator()(const vint<N>& p) const;

    inline V*& operator[](int r);
    inline V* const & operator[](int r) const;
    
    inline V linear_interpolate(const vfloat<N>& p) const;

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

    inline int coords_to_offset(const vint<N>& p) const;

    inline bool has(const coord_type& p) const { return ptr_->domain_.has(p); }
    inline bool has(const V* p) const { return p >= ptr_->data_ and p < ptr_->data_end_; }
    inline bool has_data() const { return !!ptr_; }
    // Access to raw buffer.
    inline V* data() { return ptr_->data_; }
    inline const V* data() const { return ptr_->data_; }
    inline const V* data_end() const { return ptr_->data_end_; }

    inline iterator begin() { return iterator(*ptr_->domain_.begin(), *this); }
    inline iterator end() { return iterator(*ptr_->domain_.end(), *this); }

    inline int pitch() const { return ptr_->pitch_; }
    inline int border() const { return ptr_->border_; }
    inline int alignment() const { return ptr_->alignment_; }

    template <typename U>
    inline imageNd<U, N>& cast() { return *(imageNd<U, N>*)this; }
    template <typename U>
    inline const imageNd<U, N>& cast() const { return *(const imageNd<U, N>*)this; }

    // Domain
    inline const boxNd<N>& domain() const { return ptr_->domain_; }
    inline boxNd<N> domain_with_border() const { return ptr_->domain_ + vpp::border(ptr_->border_); }

    inline const vector<int, N>& first_point_coordinates() const { return ptr_->domain_.first_point_coordinates(); }

    inline const vector<int, N>& last_point_coordinates() const { return ptr_->domain_.last_point_coordinates(); }

    inline self subimage(const boxNd<N>& d);
    inline const self const_subimage(const boxNd<N>& d) const;

    inline void set_external_data_holder(void* data, void (data_deleter)(void*))
    { ptr_->data_sptr_ = std::shared_ptr<void>(data, data_deleter); }

    inline void swap(imageNd<V, N>& o) { o.ptr_.swap(ptr_); }

  protected:
    template <typename... O>
    void allocate(const std::vector<int>& dims, const iod::sio<O...>& options);
    void index_rows();
    std::shared_ptr<imageNd_data<V, N> > ptr_;
  };

  template <typename V, unsigned N>
  using shared_imageNd = std::shared_ptr<imageNd<V, N> >&;

  template <typename V, unsigned N>
  imageNd<V, N> operator|(imageNd<V, N>& img, const boxNd<N>& b) { return img.subimage(b); }

  template <typename V, unsigned N>
  const imageNd<V, N> operator|(const imageNd<V, N>& img, const boxNd<N>& b) { return img.const_subimage(b); }

};

# include <vpp/core/imageNd.hpp>

#endif
