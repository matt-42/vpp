
#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<vuchar3> i1(100, 100);

  unsigned char i = 0;
  for (vint2 p : i1.domain())
  {
    i1(p) = vuchar3{i, i, i};
    i++;
  }

  image2d<vuchar1> i2 = rgb_to_graylevel<vuchar1>(i1);
  i = 0;
  for (vint2 p : i1.domain())
  {
    assert(i2(p)[0] == i);
    i++;
  }

}
