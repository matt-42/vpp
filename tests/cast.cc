#include <iostream>
#include <vpp/vpp.hh>

using namespace vpp;

int main()
{
  //typename std::enable_if<Eigen::MatrixBase<vint1>::SizeAtCompileTime != 1>::type xx = 2;

  vuchar3 p;
  const Eigen::MatrixBase<vuchar3>& ref = p;

  assert(p == ref);

  std::cout << !std::is_base_of<Eigen::EigenBase<vfloat1>, vfloat1>::value << std::endl;
  vint3 v = cast<vint3>(p);
  vint3 x = cast<vint3>(p - p);
  vint1 y = cast<vint1>(vfloat1(1));

  int z = cast<int>(vfloat1(1));
  vint1 a = cast<vint1>(float(1));

  // vint1 xxx; xxx[0] = (20);
  // float y = cast<float>(xxx);
  // assert(y == 20);

  // vint1 vi = cast<vint1>(float(3.f));
  // assert(vi[0] == 3);
}
