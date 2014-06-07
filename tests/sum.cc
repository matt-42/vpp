#include <iostream>
#include <vpp/core/image2d.hh>
#include <vpp/core/sum.hh>

int main()
{
  using namespace vpp;

  image2d<char> img(100, 200);

  int s = 0;
  char i = 0;
  for (char& c : img) { c = i++;  s += c; }

  assert(sum(img) == s);

}
