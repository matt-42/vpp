#include <iostream>
#include <vpp/image2d.hh>

int main()
{
  using vpp::image2d;
  using vpp::vint2;
  using vpp::make_box2d;

  image2d<int> img1(make_box2d(100, 200));
  image2d<int> img2({100, 200});

  assert(img1.domain() == img2.domain());

  assert(img1.nrows() == 100);
  assert(img1.ncols() == 200);

}
