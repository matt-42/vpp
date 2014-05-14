#include <iostream>
#include <vpp/image2d.hh>
#include <vpp/fill.hh>

int main()
{
  using vpp::imageNd;
  using vpp::fill;

  imageNd<int, 2> img({100, 200});

  fill(img, 42);

  for (auto& v : img) assert(v == 42);

}
