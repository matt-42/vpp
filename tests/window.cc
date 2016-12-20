#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> A(100, 100);

  auto win = make_window([] () {
      return make_array(
    vint2{-1, -1}, vint2{-1, 0}, vint2{-1, 1},
    vint2{0, -1}, vint2{0, 0}, vint2{0, 1},
    vint2{1, -1}, vint2{1, 0}, vint2{1, 1}); });

  int& pix = A(5, 5);

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
  auto ra = relative_accessor(A, vint2(5,5));

  foreach(win, [&] (vint2 n) { ra(n) = i++; });

  i = 0;
  for (int dr = 4; dr <= 6; dr++)
  for (int dc = 4; dc <= 6; dc++)
  {
    assert(A(dr, dc) == i);
    i++;
  }
  
  assert(i == 9);
}


