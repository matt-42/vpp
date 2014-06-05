#include <iostream>
#include <vpp/core/image2d.hh>
#include <vpp/core/pyramid.hh>

int main()
{
  using namespace vpp;

  image2d<vint1> img(4, 4);

  int s1[] = {
    1,2,3,4,
    5,6,7,8,
    9,10,11,12,
    13,14,15,16
  };

  int s2[] = {
    (1+2+5+6)/4, (3+4+7+8)/4,
    (9+10+13+14)/4, (11+12+15+16)/4
  };

  int i = 0;
  for (vint1& p : img) p[0] = s1[i++];

  pyramid2d<vint1> pyramid(img.domain(), 2, 2);

  assert(pyramid.factor() == 2);
  assert(pyramid.levels().size() == 2);
  assert(pyramid.levels()[0].domain() == img.domain());
  assert(pyramid.levels()[1].domain() == make_box2d(2, 2));

  pyramid.update(img);

  i = 0;
  for (const vint1& p : pyramid.levels()[1])
  {
    assert(p[0] == s2[i++]);
  }

}
