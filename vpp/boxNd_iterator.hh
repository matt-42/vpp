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

    boxNd_iterator(const coord_type cur, const boxNd<N, C>& box);

    boxNd_iterator<N, C>& next();
    boxNd_iterator<N, C>& operator++() { return next(); }

    operator coord_type() const;
    coord_type operator*() const;

  private:
    const boxNd<N, C>& box_;
    coord_type cur_;
  };

  template <unsigned N, typename C>
  bool operator==(const boxNd_iterator<N, C>& a, const boxNd_iterator<N, C>& b)
  {
    return *a == *b;
  }

  template <unsigned N, typename C>
  bool operator!=(const boxNd_iterator<N, C>& a, const boxNd_iterator<N, C>& b)
  {
    return *a != *b;
  }
};

# include <vpp/boxNd_iterator.hpp>

#endif
