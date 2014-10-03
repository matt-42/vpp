#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> A(1000, 1000);
  image2d<int> B(1000, 1000, border(2));
  image2d<int> C(1000, 1000, border(2));

  pixel_wise(A)(row_forward) < [] (auto& a) { a = 2; };

  // auto Bnbh = make_box_nbh2d<2,2>(B);
  // auto Cnbh = make_box_nbh2d<2,2>(C);
  // pixel_wise(A, Bnbh, Cnbh, row_first, column_first) < [] (auto& a,auto& b,auto& c) {
  //   b(0,0) = b(0,-1) + a;
  //   c(0,0) = c(-1,0) + b(0,0);
  // };

  // row_first, one_thread_per_row
  // iterate_on(A, B, C)(parallel, backward, colunm_wise, static_schedule = 5) < [] (auto& o)
  // {
  //   o.a = o.b + o.c;
  // };

  // for (int i = 0; i < A.nrows(); i++) {
  //   A
  // }

  // col_wise(A, Bnbh) << [] (auto& a, auto& b) { b(0,0) = b(-1, 0) + a; };
  // row_wise(B, Cnbh) << [] (auto& b, auto& c) { c(0,0) = c(0, -1) + b; };

  // pixel_wise(
  //   images = ( a = pixel_accessor(A),
  //              b = pixel_accessor(B),
  //              c = neighbor_accessor<5, 5>(C)),
  //   parallel = true,
  //   dependency = row_forward) | f;

}
