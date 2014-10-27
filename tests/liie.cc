#include <iostream>
#include <vpp/vpp.hh>

#include <iod/sio.hh>
#include <iod/sio_utils.hh>
#include <iod/foreach.hh>
#include <iod/grammar.hh>
#include <iod/grammar_utils.hh>

#include <vpp/core/liie.hh>


using namespace vpp;
using namespace vpp::liie;

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

  
  auto X = pixel_wise(_V(B) + _V(A));
  pixel_wise(X.domain(), X)(_No_threads) | [] (vint2 p, int x) {
    if (x != 3) std::cout << p.transpose() << " " << x << std::endl;
  };

  //pixel_wise((_V(X)) = (_V(A)));
  //pixel_wise(X) | [] (int x) { assert(x == 1); };

  pixel_wise(_V(X) = _V(A) + _V(B));
  pixel_wise(X) | [] (int x) { assert(x == 3); };
}
