#include <iostream>
#include <vpp/parallel_for.hh>
#include <vpp/image2d.hh>

int main()
{
  using vpp::parallel_for_openmp;
  using vpp::image2d;
  using vpp::vint2;
  using vpp::box2d;

  image2d<int> img2(50, 300);

  parallel_for_openmp(img2.domain()) << [&] (vint2 p)
  {
    img2(p) = 42;
  };

  for (auto p : img2.domain())
    assert(img2(p) == 42);

  // pixel wise
  vpp::pixel_wise(img2) << [&] (auto& p) { p = 43; };

  for (auto p : img2.domain()) { assert(img2(p) == 43); }

  // row wise
  vpp::row_wise(img2) << [&] (auto& p) { p = 44; };
  for (auto p : img2.domain()) {
    if (p[1] == 0) assert(img2(p) == 44);
    else
      assert(img2(p) == 43);
  }

  // One domain.
  box2d domain = vpp::make_box2d(10, 10);
  auto it = domain.begin();
  vpp::pixel_wise(domain) << [&] (auto& p) {
    assert(*it == p);
    it.next();
  };

  // col_wise
  vpp::col_wise(img2) << [&] (auto& p) { p = 45; };
  for (auto p : img2.domain()) {
    if (p[0] == 0) assert(img2(p) == 45);
    else
      assert(img2(p) != 45);
  }

}
