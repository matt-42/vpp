#include <iostream>
#include <vpp/image2d.hh>
#include <vpp/fill.hh>

int main()
{
  using vpp::imageNd;
  using vpp::fill;

  int dims[] = {100, 200};
  imageNd<int, 2> img(dims);

  fill(img, 42);

  for (auto& v : img) assert(v == 42);

}
