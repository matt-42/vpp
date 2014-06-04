#ifndef VPP_IMAGEND_ITERATOR_HH_
# define VPP_IMAGEND_ITERATOR_HH_

namespace vpp
{

  template <typename V, unsigned N>
  class imageNd;
  template <typename V, unsigned N>
  class imageNd_iterator;
  template <typename V, unsigned N>
  class imageNd_iterator2;

  template <typename V, typename C>
  class pixel
  {
  public:

    inline pixel() {}
    inline pixel(V* val, const C& c) : val_(val), coord_(c) {}

    template <typename U>
    inline pixel& operator=(const U& u) { (*val_) = u; return *this; }
    inline const V& operator*() const { return *val_; }
    inline V& operator*() { return *val_; }
    inline const V* addr() const { return val_; }
    inline V* addr() { return val_; }

    inline const C& coord() const { return coord_; }

  private:
    V* val_;
    C coord_;

    friend class imageNd_iterator<V, C::RowsAtCompileTime>;
    friend class imageNd_iterator2<V, C::RowsAtCompileTime>;
  };

  template <typename V, unsigned N>
  class imageNd_row_iterator
  {
  public:
    typedef imageNd<V, N> I;
    typedef typename I::coord_type coord_type;
    typedef typename I::value_type value_type;
    typedef pixel<value_type, coord_type> pixel_type;

    typedef typename I::domain_type::row_iterator bi_type;
    typedef imageNd_row_iterator<V, N> self;

    inline imageNd_row_iterator()
    {}

    inline imageNd_row_iterator(const imageNd_row_iterator<V, N>& o)
    {
      *this = o;
    }

    inline imageNd_row_iterator<V, N>& operator=(const imageNd_row_iterator<V, N>& o)
    {
      cur_ = o.cur_;
      return *this;
    }

    inline imageNd_row_iterator(const coord_type& cur, I& image)
      : cur_(image.address_of(cur))
    {}

    inline void next()
    {
      cur_ += 1;
    }

    inline self& operator++() { next(); return *this; }

    inline const value_type& operator*() const { return *cur_; }
    inline const value_type* operator->() const { return cur_; }

    inline value_type& operator*() { return *cur_; }
    inline value_type* operator->() { return cur_; }

  private:
    V* cur_;
  };

  template <typename V, unsigned N>
  inline bool operator==(const imageNd_row_iterator<V, N>& a, const imageNd_row_iterator<V, N>& b)
  {
    return &*a == &*b;
  }

  template <typename V, unsigned N>
  inline bool operator!=(const imageNd_row_iterator<V, N>& a, const imageNd_row_iterator<V, N>& b)
  {
    return !(a == b);
  }


  template <typename V, unsigned N>
  class imageNd_iterator
  {
  public:
    typedef imageNd<V, N> I;
    typedef typename I::coord_type coord_type;
    typedef typename I::value_type value_type;
    typedef pixel<value_type, coord_type> pixel_type;

    typedef typename I::domain_type::iterator bi_type;
    typedef imageNd_iterator<V, N> self;

    inline imageNd_iterator()
    {}

    inline imageNd_iterator(const imageNd_iterator<V, N>& o)
    {
      *this = o;
    }

    inline imageNd_iterator<V, N>& operator=(const imageNd_iterator<V, N>& o)
    {
      pitch_ = o.pitch_;
      row_spacing_ = o.row_spacing_;
      cur_ = o.cur_;
      eor_ = o.eor_;
    }

    inline imageNd_iterator(const coord_type& cur, I image)
      : pitch_(image.pitch()),
        row_spacing_(image.pitch() - image.ncols() * sizeof(V)),
        cur_(image.address_of(cur)),
        eor_(cur_ + image.ncols())
    {}

    inline void next()
    {
      cur_++;
      if (cur_ == eor_)
      {
        cur_ = (V*)((char*) cur_ + row_spacing_);
        eor_ = (V*)((char*) eor_ + pitch_);
      }
    }

    inline self& operator++() { next(); }

    inline const value_type& operator*() const { return *cur_; }
    inline const value_type* operator->() const { return cur_.addr(); }

    inline value_type& operator*() { return *cur_; }
    inline value_type* operator->() { return cur_.addr(); }

  private:
    int pitch_;
    int row_spacing_;
    V* cur_;
    V* eor_;
  };

  template <typename V, unsigned N>
  inline bool operator==(const imageNd_iterator<V, N>& a, const imageNd_iterator<V, N>& b)
  {
    return &*a == &*b;
  }

  template <typename V, unsigned N>
  inline bool operator!=(const imageNd_iterator<V, N>& a, const imageNd_iterator<V, N>& b)
  {
    return !(a == b);
  }

};

#endif
