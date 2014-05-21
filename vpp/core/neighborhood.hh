#ifndef VPP_NEIGHBORHOOD_HH__
# define VPP_NEIGHBORHOOD_HH__

# include <vpp/core/boxNd.hh>
# include <vpp/core/vector.hh>

namespace vpp
{

  // template <unsigned N, typename C>
  // class box_neighborhood_iterator
  // {
  // public:
  //   typedef vector<C, N> coord_type;

  //   box_neighborhood_iterator(const coord_type& p, const coord_type& cur, const boxNd<N, C>& box)
  //     : p_(p),
  //       box_it_(cur, box)
  //   {}

  //   box_neighborhood_iterator& operator++() { box_it_.next(); return *this; }

  //   operator coord_type() const { return p_ + *box_it_; }
  //   coord_type operator*() const { return p_ + *box_it_; }

  //   const boxNd_iterator<N, C>& box_it() const { return box_it_; }

  // private:
  //   const coord_type& p_;
  //   boxNd_iterator<N, C> box_it_;
  // };

  // template <unsigned N, typename C>
  // bool operator==(const box_neighborhood_iterator<N, C>& a, const box_neighborhood_iterator<N, C>& b)
  // {
  //   return a.box_it() == b.box_it();
  // }

  // template <unsigned N, typename C>
  // bool operator!=(const box_neighborhood_iterator<N, C>& a, const box_neighborhood_iterator<N, C>& b)
  // {
  //   return !(a == b);
  // }

  // template <unsigned N, typename C>
  // class box_neighborhood
  // {
  // public:
  //   typedef vector<C, N> coord_type;

  //   box_neighborhood(coord_type p, int width)
  //     : p_(p)
  //   {
  //     coord_type p1, p2;
  //     for (unsigned i = 0; i < N; i++)
  //     {
  //       p1[i] = -width / 2;
  //       p2[i] = width / 2;
  //     }
  //     box_.p1() = p1;
  //     box_.p2() = p2;
  //   }

  //   box_neighborhood_iterator<N, C> begin() { return box_neighborhood_iterator<N, C>(p_, box_.p1(), box_); }
  //   box_neighborhood_iterator<N, C> end() { return box_neighborhood_iterator<N, C>(p_, *box_.end(), box_); }

  // private:
  //   coord_type p_;
  //   boxNd<N, C> box_;
  // };

  // template <typename V>
  // box_neighborhood<V> box_nbh_2d(image2d<V>& p, int nrows, int ncols)
  // {
  //   return box_neighborhood<C::RowsAtCompileTime, typename C::Scalar>(p, width);
  // }

  // box_neighborhood<2, int> box_nbh(const vint2& p, int width)
  // {
  //   return box_neighborhood<2, int>(p, width);
  // }

  // template <unsigned N>
  // class neighborhood_iterator
  // {
  // public:

  //   neighborhood_iterator(i_int2 p, box2d b);
  //   neighborhood_iterator(const neighborhood_iterator& mit);
  //   neighborhood_iterator& operator++();
  //   neighborhood_iterator operator++(int);
  //   bool operator==(const neighborhood_iterator& rhs);
  //   bool operator!=(const neighborhood_iterator& rhs);
  //   const i_int2& operator*();
  // };

  // template <typename N, unsigned D>
  // neighborhood_iterator neighborhood(vint<D> p, N nhb);

  // vint2 c9[] = {
  //   [-1,-1], [-1, 0], [-1, 1],
  //   [ 0, -1], [ 0, 0], [0, 1],
  //   [1,-1], [1, 0], [1, 1]
  // };

  // vint2 c8[] = {
  //   [-1,-1], [-1, 0], [-1, 1],
  //   [ 0, -1], [0, 1],
  //   [1,-1], [1, 0], [1, 1]
  // };

  // vint2 c5[] = {
  //   [-1, 0],
  //   [ 0, -1], [ 0, 0], [0, 1],
  //   [1, 0]
  // };

  // vint2 c4[] = {
  //   [-1, 0],
  //   [ 0, -1], [0, 1],
  //   [1, 0]
  // };

};

#endif
