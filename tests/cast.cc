#include <iostream>
#include <vpp/vpp.hh>

using namespace vpp;

int main()
{
  vuchar3 p;
  vint3 x = cast<vint3>(p);

  vint1 xxx; xxx[0] = (20);
  float y = cast<float>(xxx);
  assert(y == 20);

  vint1 vi = cast<vint1>(float(3.f));
  assert(vi[0] == 3);
}
