#ifndef VPP_BOXND_ITERATOR_HH_
# define VPP_BOXND_ITERATOR_HH_

namespace vpp
{

  template <unsigned N, typename C>
  class boxNd;

  template <unsigned N, typename C = int>
  class boxNd_iterator
  {
  public:
    typedef vector<C, N> coord_type;
    typedef boxNd_iterator<N, C> self;

    inline boxNd_iterator() {}
    inline boxNd_iterator(const coord_type cur, const boxNd<N, C>& box);

    inline boxNd_iterator(const self& o)
    { box_ = o.box_; cur_ = o.cur_; }

    inline boxNd_iterator<N, C>& next();
    inline boxNd_iterator<N, C>& operator++() { return next(); }

    inline operator coord_type() const;
    inline const coord_type& operator*() const;

  private:
    const boxNd<N, C>* box_;
    coord_type cur_;
    //int line_end_;
    // C& last_c_;
  };

  template <unsigned N, typename C>
  inline bool operator==(const boxNd_iterator<N, C>& a, const boxNd_iterator<N, C>& b)
  {
    return *a == *b;
  }

  template <unsigned N, typename C>
  inline bool operator!=(const boxNd_iterator<N, C>& a, const boxNd_iterator<N, C>& b)
  {
    return *a != *b;
  }


  template <unsigned N, typename C = int>
  class boxNd_row_iterator
  {
  public:
    typedef vector<C, N> coord_type;
    typedef boxNd_row_iterator<N, C> self;

    inline boxNd_row_iterator() {}
    inline boxNd_row_iterator(const coord_type cur, const boxNd<N, C>&) : cur_(cur) {}

    inline boxNd_row_iterator(const self& o)
    { cur_ = o.cur_; }

    inline boxNd_row_iterator<N, C>& next() { cur_[N-1]++; return *this; }
    inline boxNd_row_iterator<N, C>& prev() { cur_[N-1]--; return *this; }
    inline boxNd_row_iterator<N, C>& operator++() { return next(); }
    inline boxNd_row_iterator<N, C>& operator--() { return prev(); }

    inline operator coord_type() const { return cur_; }
    inline const coord_type& operator*() const  { return cur_; }

  private:
    coord_type cur_;
  };

  template <unsigned N, typename C>
  inline bool operator==(const boxNd_row_iterator<N, C>& a, const boxNd_row_iterator<N, C>& b)
  {
    return *a == *b;
  }

  template <unsigned N, typename C>
  inline bool operator!=(const boxNd_row_iterator<N, C>& a, const boxNd_row_iterator<N, C>& b)
  {
    return *a != *b;
  }

};

# include <vpp/core/boxNd_iterator.hpp>

#endif
