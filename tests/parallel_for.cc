#include <iostream>
#include <vpp/vpp.hh>

using namespace vpp;

// const / non const test.
// void test_const(const image2d<int>& A, image2d<int>& B)
// {
//   vpp::pixel_wise(A, B) << [&] (const int& a, int& b) { b = a; };
// }

int main()
{
  using namespace vpp;

  image2d<int> img2(50, 300, border(1));

  vpp::pixel_wise(img2) << [&] (auto& p) { p = 42; };

  for (auto p : img2.domain())
    assert(img2(p) == 42);

  // pixel wise
  //vpp::pixel_wise(img2)(vpp::serial) << [&] (auto& p) { std::cout << p.transpose() << std::endl; p = 43; };

  fill(img2, 0);
  vpp::pixel_wise(img2) << [&] (auto& p) { p = 43; };
  for (auto p : img2.domain()) { assert(img2(p) == 43); }

  // row wise
  // vpp::row_wise(img2) << [&] (auto& p) { p = 44; };
  // for (auto p : img2.domain()) {
  //   if (p[1] == 0) assert(img2(p) == 44);
  //   else
  //     assert(img2(p) == 43);
  // }

  // One domain.
  box2d domain = vpp::make_box2d(10, 10);
  auto it = domain.begin();
  vpp::pixel_wise(domain) < [&] (auto& p) {
    assert(*it == p);
    it.next();
  };

  // Row backward.
  fill_with_border(img2, 0);
  auto nbh = box_nbh2d<int, 3, 3>(img2);

  // Row forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, nbh)(row_forward) < [&] (auto& o, auto& nbh) {
    o = o + nbh(0, -1);
  };
  for (auto p : img2.domain()) { assert(img2(p) == p[1] + 1); }

  // Row backward.
  fill(img2, 1);
  vpp::pixel_wise(img2, nbh)(row_backward) < [&] (auto& o, auto& nbh) {
    o = o + nbh(0, 1);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (img2.ncols() - p[1])); }

  // Col forward.
  fill(img2, 1);
  vpp::pixel_wise(img2, nbh)(col_forward) < [&] (auto& o, auto& nbh) {
    o = o + nbh(-1, 0);
  };
  for (auto p : img2.domain()) { assert(img2(p) == (p[0] + 1)); }

  // Col backward.
  // fill(img2, 1);
  // vpp::pixel_wise(img2, nbh)(row_backward) < [&] (auto& o, auto& nbh) {
  //   o = o + nbh(1, 0);
  //   std::cout << nbh(1, 0) << std::endl;
  // };
  // for (auto p : img2.domain()) { assert(img2(p) == (img2.nrows() - p[0])); }

  // col_wise
  // vpp::col_wise(img2) << [&] (auto& p) { p = 45; };
  // for (auto p : img2.domain()) {
  //   if (p[0] == 0) assert(img2(p) == 45);
  //   else
  //     assert(img2(p) != 45);
  // }

}
