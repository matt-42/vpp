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
      box_it_ = o.box_it_; cur_ = o.cur_;
    }

    inline imageNd_iterator(const coord_type& cur, I image)
      : box_it_(cur, image.domain()),
        cur_(image.address_of(cur), *box_it_)
    {}

    inline void next()
    {
      box_it_.next();
      cur_.val_++;
      cur_.coord_ = *box_it_;
    }

    inline self& operator++() { next(); }

    inline const value_type& operator*() const { return *cur_; }
    inline const value_type* operator->() const { return cur_.addr(); }

    inline value_type& operator*() { return *cur_; }
    inline value_type* operator->() { return cur_.addr(); }

  private:
    bi_type box_it_;
    pixel_type cur_;
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
      cur_++;
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


  // Try to optimize imageNd_iterator. Need more benchmarking on different archtectures.
  template <typename V, unsigned N>
  class imageNd_iterator2
  {
  public:
    typedef imageNd<V, N> I;
    typedef typename I::coord_type coord_type;
    typedef typename I::value_type value_type;
    typedef pixel<value_type, coord_type> pixel_type;

    typedef typename I::domain_type::iterator bi_type;
    typedef imageNd_iterator2<V, N> self;

    inline imageNd_iterator2(const coord_type cur, I& image)
      : box_(image.domain()),
        cur_(&image(cur), cur)
    {}

    void next_line()
    {
      int n = N;

      while (--n > 0 and cur_.coord_[n] == (box_.p2()[n] + 1))
      {
        cur_.coord_[n] = box_.p1()[n];
        cur_.coord_[n - 1]++;
      }
      row_end_ = cur_.val_ + box_.size(N - 1);
    }

    inline self& next()
    {
      cur_.val_++;
      cur_.coord_[N - 1]++;
      if (cur_.val_ == row_end_)
        next_line();
      return *this;
    }

    inline self& operator++() { return next(); }

    inline const pixel_type& operator*() const { return cur_; }
    inline const pixel_type* operator->() const { return &cur_; }

    inline pixel_type& operator*() { return cur_; }
    inline pixel_type* operator->() { return &cur_; }

  private:
    typename I::domain_type box_;
    V* row_end_;
    pixel_type cur_;
  };

  template <typename V, unsigned N>
  inline bool operator==(const imageNd_iterator2<V, N>& a, const imageNd_iterator2<V, N>& b)
  {
    return a->addr() == b->addr();
  }

  template <typename V, unsigned N>
  inline bool operator!=(const imageNd_iterator2<V, N>& a, const imageNd_iterator2<V, N>& b)
  {
    return a->addr() != b->addr();
  }

};

#endif
