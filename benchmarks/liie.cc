#include <iostream>
#include <vpp/vpp.hh>
#include <vpp/core/liie.hh>

#include "get_time.hh"

using namespace vpp;
using namespace vpp::liie;

// void operator+(const image2d<int>& l, const image2d<int>& r)
// {
//   std::cout << "A: " << l.data() << std::endl;
//   std::cout << "B: " << r.data() << std::endl;
  
// }

int main()
{
  image2d<int> A(1000,1000);
  image2d<int> B(1000,1000);
  
  // A + B;
  fill(A, 1);
  fill(B, 2);

  auto time = get_time_in_seconds();

  auto res = eval(_v(A) + _v(B));
  for (int K = 0; K < 1000; K++)
    pixel_wise(res, A, B) | [] (auto& r, auto& a, auto& b) { r = a + b; };

  std::cout << 1000 * (get_time_in_seconds() - time) << std::endl;

  time = get_time_in_seconds();

  for (int K = 0; K < 1000; K++)
    pixel_wise(_v(res) = _v(A) + _v(B));

  std::cout << 1000 * (get_time_in_seconds() - time) << std::endl;

  
}
