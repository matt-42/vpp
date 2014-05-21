#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using vpp::image3d;
  using vpp::vint3;
  using vpp::make_box3d;

  image3d<int> img1(make_box3d(100, 200, 300));
  image3d<int> img2({100, 200, 300});

  assert(img1.domain() == img2.domain());

  assert(img1.nslices() == 100);
  assert(img1.nrows() == 200);
  assert(img1.ncols() == 300);

}
