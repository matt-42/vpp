#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> img(4,4);
  vint2 b(2,2);
  int i = 0;
  block_wise(b, img, img) | [&] (image2d<int> I, image2d<int> J)
  {
    assert(I.nrows() == b[0]);
    assert(I.ncols() == b[1]);
    fill(I, i);
    i++;
  };

  pixel_wise(img.domain(), img) | [&] (vint2 c, int& v)
  {
    c[0] /= b[0];
    c[1] /= b[1];
    int idx = (b[1] * c[0] + c[1]);
    assert(idx == v);
  };

}
