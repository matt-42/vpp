#include <iostream>
#include <vpp/vpp.hh>

#include <iod/sio.hh>
#include <iod/sio_utils.hh>
#include <iod/foreach.hh>
#include <iod/grammar.hh>
#include <iod/grammar_utils.hh>
#include <iod/symbols.hh>

#include <vpp/core/liie.hh>


using namespace vpp;
using namespace vpp::liie;
using namespace s;

int main()
{
  image2d<int> A(10,10);
  image2d<int> B(10,10);
  const image2d<int> C(10,10);

  // A + B;
  fill(A, 1);
  fill(B, 2);

  A(5,5) = 0;

  // Test compilation.
  auto X = pixel_wise(A, B) | (_Sum(_1 + _2 * 2) + _Avg(_2) + _Max(_1) + _Argmin(_1)[0]);

  // auto t = std::make_tuple(A);
  // auto e = _Argmin(_1)[0];
  // auto v = evaluate_global_expressions(e, t);
  //void* x = v;
  //std::cout << v << std::endl;
  //assert(v == 5);

  std::cout << X(0,0) << std::endl;
  fill(A, 1); fill(B, 2);
  auto Y = pixel_wise(A, B) | (_1 + _2 * 2);

  pixel_wise(Y) | [] (int y) { assert(y == 5); };
  
  fill(A, 1);
  fill(B, 2);
  auto x = pixel_wise(X, A, B) | (_1 = _2 + _3 * 3);
  std::cout << X(0,0) << std::endl;
  
  pixel_wise(X) | [] (int x) { assert(x == 7); };
}
