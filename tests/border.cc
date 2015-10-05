#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  image2d<int> img1(5, 10, _border = 2, _aligned = 1);

  // Fill with border
  {
    fill_with_border(img1, 42);

    pixel_wise(img1.domain_with_border(), img1) | [&] (vint2 p, auto v) {
      assert(v == 42);
    };
  }
  // Fill border with value.
  {
    fill(img1, 5);
    fill_border_with_value(img1, 6);

    for (int r = -2; r < 7; r++)
    {
      for (int c = -2; c < 12; c++)
        std::cout << img1(r, c) << " ";
      std::cout << std::endl;
    }

    pixel_wise(img1.domain_with_border(), img1) | [&] (vint2 p, auto v) {
      if (img1.domain().has(p)) assert(v == 5);
      else assert(v == 6);
    };
  }

  // Fill border closest.
  {
    fill_with_border(img1, 0);

    pixel_wise(img1.domain(), img1) | [&] (vint2 p, auto& v) {
      v = (p[0] + p[1]) % 10;
    };
    
    fill_border_closest(img1);

    std::cout << std::endl;
    for (int r = -2; r < 7; r++)
    {
      for (int c = -2; c < 12; c++)
        std::cout << img1(r, c) << " ";
      std::cout << std::endl;
    }

    pixel_wise(img1.domain_with_border(), img1) | [&] (vint2 p, auto& v) {
      int cc = std::max(std::min(img1.ncols() - 1, p[1]), 0);
      int cr = std::max(std::min(img1.nrows() - 1, p[0]), 0);
      assert(v == (cc + cr) % 10);
    };
    
  }

  // Fill border mirror
  {
    fill_with_border(img1, 0);

    pixel_wise(img1.domain(), img1) | [&] (vint2 p, auto& v) {
      v = (p[0] + p[1]) % 10;
    };
    
    fill_border_mirror(img1);

    std::cout << std::endl;
    for (int r = -2; r < 7; r++)
    {
      for (int c = -2; c < 12; c++)
        std::cout << img1(r, c) << " ";
      std::cout << std::endl;
    }
    
  }
}
