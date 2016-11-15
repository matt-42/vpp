#include <iostream>
#include <vpp/vpp.hh>

#include "get_time.hh"

int main()
{
  using namespace vpp;

  image2d<int> img(1000,1000);

  // Cache warm up.
  for (auto p : img.domain())
    img(p) = p[0] + p[1];

  double time;

  time = get_time_in_seconds();
  for (int k = 0; k < 40; k++)
  {
    for (int r = 0; r < 1000; r++)
    {
      int* row = &img(vint2(r, 0));
      int* end = &img(vint2(r, 1000));
      int c = 0;
      while (row != end)
      {
        *(row++) = r + c;
        c++;
      }
    }
  }
  double ref_time = get_time_in_seconds() - time;

  time = get_time_in_seconds();
  for (int k = 0; k < 40; k++)
  for (auto p : img.domain())
    img(p) = p[0] + p[1];
  double vpp_time = get_time_in_seconds() - time;

  std::cout << "iterator on domain: " << std::endl;
  std::cout << "ref_time: " << ref_time << std::endl;
  std::cout << "vpp_time: " << vpp_time << std::endl;
  std::cout << "iteration on domain overhead: " << 100. * vpp_time / ref_time - 100. << "%" << std::endl;
}
