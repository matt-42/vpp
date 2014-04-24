#ifndef VPP_BOXND_ITERATOR_HPP_
# define VPP_BOXND_ITERATOR_HPP_

namespace vpp
{

  template <unsigned N, typename C>
  boxNd_iterator<N, C>::boxNd_iterator(const coord_type cur, const boxNd<N, C>& box)
    : cur_(cur),
      box_(box)
  {}

  template <unsigned N, typename C>
  boxNd_iterator<N, C>&
  boxNd_iterator<N, C>::next()
  {
    cur_[N-1]++;

    if (cur_[N - 1] == (box_.p2()[N - 1] + 1))
    {
      int n = N;
      while (--n > 0 and !box_.has(cur_))
      {
        cur_[n] = box_.p1()[n];
        cur_[n - 1]++;
      }
    }

    return *this;
  }

  template <unsigned N, typename C>
  boxNd_iterator<N, C>::operator coord_type() const
  {
    return cur_;
  }


  template <unsigned N, typename C>
  typename boxNd_iterator<N, C>::coord_type
  boxNd_iterator<N, C>::operator*() const
  {
    return cur_;
  }


};

# include <vpp/boxNd_iterator.hh>

#endif
