#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> img1(make_box2d(100, 200), _border = 3);
  image2d<int> img2({100, 200});

  assert(&img1(0,0) == &img1[0][0]);
  assert(&img1(0,0) == &(*img1.begin()));
  assert(img1.domain() == img2.domain());

  assert(img1.nrows() == 100);
  assert(img1.ncols() == 200);

  {
    image2d<int> img(make_box2d(5, 5), _border = 1);

    assert(&img(0,0) == img.address_of(vint2(0,0)));
    assert(&img(4,0) == img.address_of(vint2(4,0)));
    
    auto s1 = img.subimage(img.domain());

    assert(&s1(0,0) == s1.address_of(vint2(0,0)));
    
    for (auto p : img.domain())
      assert(img(p) == img[p[0]][p[1]]);
    for (auto p : img.domain())
      assert(img(p) == s1[p[0]][p[1]]);
  }
}
