#ifndef VPP_NEIGHBORHOOD_HH__
# define VPP_NEIGHBORHOOD_HH__

namespace vpp
{

  template <unsigned N>
  class neighborhood_iterator
  {
  public:

    neighborhood_iterator(i_int2 p, box2d b);
    neighborhood_iterator(const neighborhood_iterator& mit);
    neighborhood_iterator& operator++();
    neighborhood_iterator operator++(int);
    bool operator==(const neighborhood_iterator& rhs);
    bool operator!=(const neighborhood_iterator& rhs);
    const i_int2& operator*();
  };

  template <typename N, unsigned D>
  neighborhood_iterator neighborhood(vint<D> p, N nhb);

  vint2 c9[] = {
    [-1,-1], [-1, 0], [-1, 1],
    [ 0, -1], [ 0, 0], [0, 1],
    [1,-1], [1, 0], [1, 1]
  };

  vint2 c8[] = {
    [-1,-1], [-1, 0], [-1, 1],
    [ 0, -1], [0, 1],
    [1,-1], [1, 0], [1, 1]
  };

  vint2 c5[] = {
    [-1, 0],
    [ 0, -1], [ 0, 0], [0, 1],
    [1, 0]
  };

  vint2 c4[] = {
    [-1, 0],
    [ 0, -1], [0, 1],
    [1, 0]
  };

};

#endif
