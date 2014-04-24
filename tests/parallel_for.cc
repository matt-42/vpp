#include <iostream>
#include <vpp/parallel_for.hh>
#include <vpp/image2d.hh>

int main()
{
  using vpp::parallel_for_openmp;
  using vpp::image2d;
  using vpp::vint2;

  image2d<int> img2(500, 300);

  parallel_for_openmp(img2.domain()) << [&] (vint2 p)
  {
    img2(p) = 42;
  };

  for (auto p : img2.domain())
  {
    assert(img2(p) == 42);
  }

}
