#ifndef VPP_IMAGEND_ITERATOR_HH_
# define VPP_IMAGEND_ITERATOR_HH_

# include <vpp/core/const.hh>

namespace vpp
{

  template <typename V, unsigned N>
  class imageNd;
  template <typename V, unsigned N, template <class> class Const>
  class imageNd_iterator;

  template <typename V, unsigned N, template <class> class Const>
  class imageNd_row_iterator
  {
  public:
    typedef imageNd<V, N> I;
    typedef typename I::coord_type coord_type;
    typedef Const<typename I::value_type> value_type;

    typedef typename I::domain_type::row_iterator bi_type;
    typedef imageNd_row_iterator<V, N, Const> self;

    inline imageNd_row_iterator()
    {}

    inline imageNd_row_iterator(const self& o)
    {
      *this = o;
    }

    inline self& operator=(const self& o)
    {
      cur_ = o.cur_;
      return *this;
    }

    inline imageNd_row_iterator(const coord_type& cur, Const<I>& image)
      : cur_(image.address_of(cur))
    {}

    inline void next() { ++cur_; }
    inline void prev() { --cur_; }

    inline self& operator++() { next(); return *this; }
    inline self& operator--() { next(); return *this; }

    inline value_type& operator*() const { return *cur_; }
    inline value_type* operator->() const { return cur_; }

  private:
    value_type* cur_;
  };

  template <typename V, unsigned N, template <class> class Const>
  inline bool operator==(const imageNd_row_iterator<V, N, Const>& a, const imageNd_row_iterator<V, N, Const>& b)
  {
    return &*a == &*b;
  }

  template <typename V, unsigned N, template <class> class Const>
  inline bool operator!=(const imageNd_row_iterator<V, N, Const>& a, const imageNd_row_iterator<V, N, Const>& b)
  {
    return !(a == b);
  }


  template <typename V, unsigned N, template <class> class Const>
  class imageNd_iterator
  {
  public:
    typedef imageNd<V, N> I;
    typedef typename I::coord_type coord_type;
    typedef Const<typename I::value_type> value_type;

    typedef typename I::domain_type::iterator bi_type;
    typedef imageNd_iterator<V, N, Const> self;

    inline imageNd_iterator()
    {}

    inline imageNd_iterator(const self& o)
    {
      *this = o;
    }

    inline self& operator=(const self& o)
    {
      pitch_ = o.pitch_;
      row_spacing_ = o.row_spacing_;
      cur_ = o.cur_;
      eor_ = o.eor_;
      return *this;
    }

    inline imageNd_iterator(const coord_type& cur, Const<I>& image)
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
        cur_ = (value_type*)((Const<char>*) cur_ + row_spacing_);
        eor_ = (value_type*)((Const<char>*) eor_ + pitch_);
      }
    }

    inline self& operator++() { next(); return *this; }

    inline value_type& operator*() const { return *cur_; }
    inline value_type* operator->() const { return cur_; }

  private:
    int pitch_;
    int row_spacing_;
    value_type* cur_;
    value_type* eor_;
  };

  template <typename V, unsigned N, template <class> class Const>
  inline bool operator==(const imageNd_iterator<V, N, Const>& a, const imageNd_iterator<V, N, Const>& b)
  {
    return &*a == &*b;
  }

  template <typename V, unsigned N, template <class> class Const>
  inline bool operator!=(const imageNd_iterator<V, N, Const>& a, const imageNd_iterator<V, N, Const>& b)
  {
    return !(a == b);
  }

};

#endif
