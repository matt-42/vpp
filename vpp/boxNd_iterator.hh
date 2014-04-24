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

    inline boxNd_iterator(const coord_type cur, const boxNd<N, C>& box);

    inline boxNd_iterator<N, C>& next();
    inline boxNd_iterator<N, C>& operator++() { return next(); }

    inline operator coord_type() const;
    inline const coord_type& operator*() const;

  private:
    const boxNd<N, C>& box_;
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
};

# include <vpp/boxNd_iterator.hpp>

#endif
