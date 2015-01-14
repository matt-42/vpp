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

  auto X = pixel_wise(A, B).eval(_V(A) = _V(A) + _V(_2));

  auto Y = pixel_wise(A, B).eval(_V(A) = _V(A) - _V(_2));
  auto Z = pixel_wise(A, B).eval(_V(A) = _V(A) * _V(_2) + _V(A) + _V(A) + _V(A) + _V(A) + _V(A) + _V(A));
  auto U = pixel_wise(A, B).eval(_V(A) = _V(A) * _V(_2) + _V(A) - _V(A) - _V(A) - _V(A) - _V(A) - _V(A));

  
  // assert(eval(A, B, A, _Argmax(_V(A))) == vint2(5,5));
  // assert(eval(_Argmax(_V(A))) == vint2(5,5));
  // assert(eval(A, _Argmax(_V(_1))) == vint2(5,5));

  // fill(A, 1); fill(B, 2);
  // X = eval(_V(A) + _V(B));
  // assert(X(0,0) == 3);

  // eval(_V(X) = _V(B) * 2);
  // assert(X(0,0) == 4);

  // eval(X, _V(_1) = _V(B) * 2);
  // assert(X(0,0) == 4);

  // A(5,5) = 1000;  
  // A(2,5) = -11;  
  // assert(eval(_Max(A)) == 1000);
  // assert(eval(_Min(A)) == -11);
}
