#include <iostream>
#include <vpp/vpp.hh>

using namespace vpp;

int main()
{
  using namespace vpp;

  //image2d<int> img2(50, 300, border(1));
  image2d<int> img2(10, 10, border(1));

  vpp::pixel_wise(img2) << [&] (auto& p) { p = 42; };

  for (auto p : img2.domain())
    assert(img2(p) == 42);

  fill(img2, 0);
  vpp::pixel_wise(img2) << [&] (auto& p) { p = 43; };
  for (auto p : img2.domain()) { assert(img2(p) == 43); }

  // One domain.
  box2d domain = vpp::make_box2d(10, 10);
  auto it = domain.begin();
  vpp::pixel_wise(domain)(row_forward, col_forward) < [&] (auto& p) {
    assert(*it == p);
    it.next();
  };

  fill_with_border(img2, 0);
  auto nbh = box_nbh2d<int, 3, 3>(img2);

  // Row forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, nbh)(row_forward) << [&] (auto& o, auto& nbh) {
    o = o + nbh(0, -1);
  };
  for (auto p : img2.domain()) { assert(img2(p) == p[1] + 1); }

  // Row backward.
  fill(img2, 1);
  vpp::pixel_wise(img2, nbh)(row_backward) << [&] (auto& o, auto& nbh) {
    o = o + nbh(0, 1);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (img2.ncols() - p[1])); }

  // Col forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, nbh, img2.domain())(col_forward) << [&] (auto& o, auto& nbh, vint2 p) {
    o = o + nbh(-1, 0);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (p[0] + 1)); }

  // Col forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, nbh, img2.domain())(col_backward) << [&] (auto& o, auto& nbh, vint2 p) {
    o = o + nbh(1, 0);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (img2.nrows() - p[0])); }

}
