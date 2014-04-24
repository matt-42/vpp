#include <iostream>
#include <vpp/boxNd.hh>
#include <vpp/image2d.hh>

#include "get_time.hh"

int main()
{
  using namespace vpp;

  image2d<int> img(1000,1000);

  // Cache warm up.
  for (int k = 0; k < 10; k++)
  for (auto p : img.domain())
    img(p) = p[0] + p[1];

  double time;

  time = get_time_in_seconds();
  for (int k = 0; k < 10; k++)
  for (int r = 0; r < 1000; r++)
  for (int c = 0; c < 1000; c++)
    img(vint2(r, c)) = r + c;
  double ref_time = get_time_in_seconds() - time;


  time = get_time_in_seconds();
  for (int k = 0; k < 10; k++)
  for (auto p : img.domain())
    img(p) = p[0] + p[1];
  double vpp_time = get_time_in_seconds() - time;

  std::cout << "Box iterator: " << std::endl;
  std::cout << "ref_time: " << ref_time << std::endl;
  std::cout << "vpp_time: " << vpp_time << std::endl;
  std::cout << "box iterator overhead: " << 100. * vpp_time / ref_time - 100. << "%" << std::endl;
}
