#include <iostream>
#include <vpp/parallel_for.hh>
#include <vpp/image2d.hh>
#include <vpp/window.hh>

int main()
{
  using vpp::parallel_for_openmp;
  using vpp::image2d;
  using vpp::vint2;
  using vpp::window;
  using vpp::border;

  image2d<int> img1(50, 300, 1);
  image2d<int> img2(50, 300, 1);

  vpp::pixel_wise(img1) << [&] (auto& p) { p = (long)(&p); };

  window<image2d<int>> nbh(img1, { {0, -1}, {0, 0}, {0, 1} });

  img1(vint2(10,9)) = 1;
  img1(vint2(10,10)) = 2;
  img1(vint2(10,11)) = 3;

  {
    int c = 9;
    for (auto& n : nbh(img1(vint2(10,10))))
    {
      std::cout << n << std::endl;
      assert(&n == &img1(vint2(10,c)));
      c++;
    }
  }

  vpp::box2d d = img1.domain() - border(1);
  vpp::pixel_wise(img1, img2) << [&] (auto& in, auto& out)
  {
    int sum = 0;
    for (auto& n : nbh(in)) sum += n;
    out = sum / 3;
  };

  // parallel_for_openmp(img2.domain()) << [&] (vint2 p)
  // {
  //   img2(p) = 42;
  // };

  // for (auto p : img2.domain())
  //   assert(img2(p) == 42);

  // vpp::pixel_wise(img2) << [&] (auto p) { p = 43; };

  // for (auto p : img2.domain()) { assert(img2(p) == 43); }

  // vpp::row_wise(img2) << [&] (auto p) { p = 45; };
  // for (auto p : img2.domain()) {
  //   if (p[1] == 0) assert(img2(p) == 44);
  //   else assert(img2(p) == 43);
  // }

}
