#include <iostream>
#include <vpp/vpp.hh>

using namespace vpp;

int main()
{
  using namespace vpp;

  //image2d<int> img2(50, 300, border(1));
  image2d<int> img2(10, 10, _border = 1);

  vpp::pixel_wise(img2) | [&] (auto& p) { p = 42; };

  for (auto p : img2.domain())
    assert(img2(p) == 42);

  fill(img2, 0);
  vpp::pixel_wise(img2) | [&] (auto& p) { p = 43; };
  for (auto p : img2.domain()) { assert(img2(p) == 43); }

  // On domain.
  box2d domain = vpp::make_box2d(10, 10);
  auto it = domain.begin();
  vpp::pixel_wise(domain)(_left_to_right, _top_to_bottom, _no_threads) | [&] (auto p) {
    assert(*it == p);
    it.next();
  };

  fill_with_border(img2, 0);

  // Row forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, relative_access(img2))(_left_to_right) | [&] (auto& o, auto nbh) {
    o = o + nbh(0, -1);
  };
  for (auto p : img2.domain()) {
    assert(img2(p) == p[1] + 1);
  }

  // Row backward.
  fill(img2, 1);
  vpp::pixel_wise(img2, relative_access(img2))(_right_to_left) | [&] (auto& o, auto nbh) {
    o = o + nbh(0, 1);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (img2.ncols() - p[1])); }

  // Col forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, relative_access(img2), img2.domain())(_top_to_bottom) | [&] (auto& o, auto nbh, vint2 p) {
    o = o + nbh(-1, 0);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (p[0] + 1)); }

  // Col forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, relative_access(img2), img2.domain())(_bottom_to_top) | [&] (auto& o, auto nbh, vint2 p) {
    o = o + nbh(1, 0);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (img2.nrows() - p[0])); }

  // Image construction.
  auto img3 = pixel_wise(img2) | [] (auto& x) { return x; };
  for (auto p : img2.domain()) { assert(img2(p) == img3(p)); }
  
}
