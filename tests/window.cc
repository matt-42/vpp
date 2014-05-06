#include <iostream>
#include <vpp/image2d.hh>
#include <vpp/window.hh>

int main()
{
  using vpp::image2d;
  using vpp::vint2;
  using vpp::window;

  image2d<int> img1(3, 3);

  window<image2d<int>> nbh(img1, {
      {-1, -1}, {-1, 0}, {-1, 1},
      {0, -1}, {0, 0}, {0, 1},
      {1, -1}, {1, 0}, {1, 1}
    });

  int i = 0;
  for (auto& v : img1) v = i++;

  i = 0;
  for (auto& n : nbh(img1(vint2(1,1))))
    assert(n == i++);

}
