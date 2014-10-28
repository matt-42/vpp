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

// void operator+(const image2d<int>& l, const image2d<int>& r)
// {
//   std::cout << "A: " << l.data() << std::endl;
//   std::cout << "B: " << r.data() << std::endl;
  
// }

int main()
{
  image2d<int> A(10,10);
  image2d<int> B(10,10);
  const image2d<int> C(10,10);

  // A + B;
  fill(A, 1);
  fill(B, 2);

  A(5,5) = 0;
  
  auto X = pixel_wise(A, B) | (_Sum(_1 + _2 * 2) + _Avg(_2) + _Max(_1) + _Argmin(_1)[0]);

  // auto t = std::make_tuple(A);
  // auto e = _Argmin(_1)[0];
  // auto v = evaluate_global_expressions(e, t);
  //void* x = v;
  //std::cout << v << std::endl;
  //assert(v == 5);

  std::cout << X(0,0) << std::endl;
  //void* x = X;
  // auto X = pixel_wise(A, B)
  //   | (_S = _1 * 2,
  //      _2 = _S + 1);

  // pixel_wise(X.domain(), X)(_No_threads) | [] (vint2 p, int x) {
  //   assert(x == 5);
  // };
  
  // pixel_wise(X, A, B) | ( _1 = _2 + _3 * 3);

  // pixel_wise(X) | [] (int x) { assert(x == 7); };
}
