#include <iostream>
#include <vpp/neighborhood.hh>

int main()
{
  using vpp::vint2;
  using vpp::box_nbh;

  vint2 p(5,5);

  vint2 ref[] = {
    vint2(4, 4),
    vint2(4, 5),
    vint2(4, 6),
    vint2(5, 4),
    vint2(5, 5),
    vint2(5, 6),
    vint2(6, 4),
    vint2(6, 5),
    vint2(6, 6)
  };

  int i = 0;
  for (auto n : box_nbh(p, 3))
    assert(n == ref[i++]);

}
