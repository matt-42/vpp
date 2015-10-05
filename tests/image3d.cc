#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image3d<int> img1(make_box3d(10, 20, 30));
  image3d<int> img2(10, 20, 30);

  assert(img1.domain() == img2.domain());

  assert(img1.nslices() == 10);
  assert(img1.nrows() == 20);
  assert(img1.ncols() == 30);

  for (int s = 0; s < img1.nslices(); s++)
  for (int r = 0; r < img1.nrows(); r++)
  for (int c = 0; c < img1.ncols(); c++)
  {
    img1(s, r, c) = s * r * c;
    assert(img1(s, r, c) == (s * r * c));
  }

  // Subimage
  {
    auto s1 = img1 | box3d(vint3(2,3,4), vint3(5,6,7));
    assert(&s1(0,0,0) == &img1(vint3(2,3,4)));
    assert(&s1(0,1,1) == &img1(vint3(2,3,4) + vint3(0,1,1)));
    assert(&s1(1,1,1) == &img1(vint3(2,3,4) + vint3(1,1,1)));
    assert(&s1(2,2,2) == &img1(vint3(2,3,4) + vint3(2,2,2)));
  }
}
