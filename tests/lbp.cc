
#include <vpp/vpp.hh>
#include <vpp/algorithms/lbp/lbp_transform.hh>
#include <vpp/algorithms/lbp/lbp_distance.hh>

int main()
{
  using namespace vpp;
  
  image2d<unsigned char> V(3,3, _Border = 1);
  image2d<unsigned char> lbp(3,3);

  V(1,1) = 1;

  V(0,0) = 0;
  V(0,1) = 2;
  V(0,2) = 2;

  V(1,0) = 2;
  V(1,2) = 0;

  V(2,0) = 2;
  V(2,1) = 0;
  V(2,2) = 2;

  unsigned char x = 0b10101110;
  
  lbp_transform(V, lbp);

  unsigned char y = lbp(1,1);
  for (int i = 0; i <= 8; i++)
    std::cout << ((y & (1 << i)) >> i);
  std::cout << std::endl;

  for (int i = 0; i <= 8; i++)
    std::cout << ((x & (1 << i)) >> i);
  std::cout << std::endl;
  
  assert(lbp(1,1) == x);

  assert(lbp_hamming_distance(0b01010101, 0b01010101) == 0);
  assert(lbp_hamming_distance(0b11010101, 0b01010101) == 1);
  assert(lbp_hamming_distance(0b11111111, 0b00000000) == 8);
}
