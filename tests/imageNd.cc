#include <iostream>
#include <vpp/imageNd.hh>

int main()
{
  using vpp::imageNd;
  using vpp::vint2;

  int dims[] = {100, 200};
  imageNd<int, 2> img(dims);

  assert(img.domain().size(0) == dims[0]);
  assert(img.domain().size(1) == dims[1]);

  assert(img.data() == img.begin());

  assert(img.coords_to_index(vpp::vint2::Ones()) == dims[1] + 1);

  for (int r = 0; r < img.domain().size(0); r++)
    for (int c = 0; c < img.domain().size(1); c++)
    {
      assert(img.coords_to_index(vint2(r, c)) == (r * dims[1] + c));
    }

  for (int r = 0; r < img.domain().size(0); r++)
    for (int c = 0; c < img.domain().size(1); c++)
    {
      img(vint2(r, c)) = r * c;
    }


  for (int r = 0; r < img.domain().size(0) - 1; r++)
    for (int c = 0; c < img.domain().size(1) - 1; c++)
    {
      assert(img(vint2(r, c)) == r * c);
    }

}
