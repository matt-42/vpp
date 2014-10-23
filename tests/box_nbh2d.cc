#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> A(3, 3);
  auto nbh = box_nbh2d<int, 3,3>(A, vint2{1, 1});

  fill(A, 1);

  nbh.for_all([] (auto& p) { p = 2; });
  nbh.north() = 3;
  nbh.east() = 4;
  nbh.south() = 5;
  nbh.west() = 6;

  assert(A(0,0) == 2);
  assert(A(0,1) == 3);
  assert(A(0,2) == 2);

  assert(A(1,0) == 6);
  assert(A(1,1) == 2);
  assert(A(1,2) == 4);

  assert(A(2,0) == 2);
  assert(A(2,1) == 5);
  assert(A(2,2) == 2);
}
