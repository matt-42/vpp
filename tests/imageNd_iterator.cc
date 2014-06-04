#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> img(3, 3, border(1));

  vint2 ref[] = {
    vint2(0, 0),
    vint2(0, 1),
    vint2(0, 2),
    vint2(1, 0),
    vint2(1, 1),
    vint2(1, 2),
    vint2(2, 0),
    vint2(2, 1),
    vint2(2, 2)
  };

  int i = 0;
  for (auto& p : img)
  {
    std::cout << long(&p -  &img(0,0)) << " " << (&img(ref[i]) - &img(0,0)) << std::endl;
    assert(&p == &img(ref[i]));
    i++;
  }
}
