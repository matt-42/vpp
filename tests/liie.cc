#include <iostream>
#include <vpp/vpp.hh>

#include <iod/sio.hh>
#include <iod/sio_utils.hh>
#include <iod/foreach.hh>
#include <iod/grammar.hh>
#include <iod/symbols.hh>

#include <vpp/core/pixel_wise.hh>
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

  A(5,5) = 1000;

  auto X = pixel_wise(A, B).eval(_v(A) = _v(A) + _v(_2));

  auto Y = pixel_wise(A, B).eval(_v(A) = _v(A) - _v(_2));
  auto Z = pixel_wise(A, B).eval(_v(A) = _v(A) * _v(_2) + _v(A) + _v(A) + _v(A) + _v(A) + _v(A) + _v(A));
  auto U = pixel_wise(A, B).eval(_v(A) = _v(A) * _v(_2) + _v(A) - _v(A) - _v(A) - _v(A) - _v(A) - _v(A));

  
  // assert(eval(A, B, A, _argmax(_v(A))) == vint2(5,5));
  // assert(eval(_argmax(_v(A))) == vint2(5,5));
  // assert(eval(A, _argmax(_v(_1))) == vint2(5,5));

  // fill(A, 1); fill(B, 2);
  // X = eval(_v(A) + _v(B));
  // assert(X(0,0) == 3);

  // eval(_v(X) = _v(B) * 2);
  // assert(X(0,0) == 4);

  // eval(X, _v(_1) = _v(B) * 2);
  // assert(X(0,0) == 4);

  // A(5,5) = 1000;  
  // A(2,5) = -11;  
  // assert(eval(_max(A)) == 1000);
  // assert(eval(_min(A)) == -11);
}
