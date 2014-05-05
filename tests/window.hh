#include <iostream>
#include <vpp/parallel_for.hh>
#include <vpp/apply_tuple2.hh>
#include <vpp/apply_tuple3.hh>
#include <vpp/image2d.hh>

int main()
{
  using vpp::parallel_for_openmp;
  using vpp::image2d;
  using vpp::vint2;

  image2d<int> img2(50, 300);

  vpp::pixel_wise(img2) << [&] (auto& p) { p = (long)(&p); };

  // parallel_for_openmp(img2.domain()) << [&] (vint2 p)
  // {
  //   img2(p) = 42;
  // };

  // for (auto p : img2.domain())
  //   assert(img2(p) == 42);

  // vpp::pixel_wise(img2) << [&] (auto p) { p = 43; };

  // for (auto p : img2.domain()) { assert(img2(p) == 43); }

  // vpp::row_wise(img2) << [&] (auto p) { p = 45; };
  // for (auto p : img2.domain()) {
  //   if (p[1] == 0) assert(img2(p) == 44);
  //   else assert(img2(p) == 43);
  // }

}
