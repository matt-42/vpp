#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> A(100, 100);

  auto win = make_window(A, {
      {-1, -1}, {-1, 0}, {-1, 1},
      {0, -1}, {0, 0}, {0, 1},
      {1, -1}, {1, 0}, {1, 1}
    });

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
  win(pix) < [&] (int& p) { assert( &p == &A(ref[i++])); };
  assert(i == 9);
}


