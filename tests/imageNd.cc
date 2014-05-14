#include <iostream>
#include <vpp/imageNd.hh>

int main()
{
  using vpp::imageNd;
  using vpp::vint2;
  using vpp::vint3;

  imageNd<int, 2> img_test({2,3});


  std::vector<int> dims = {100, 200};


  imageNd<int, 2> img(dims);


  assert(img.domain().size(0) == dims[0]);
  assert(img.domain().size(1) == dims[1]);

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
      img(r, c) = r * c;
    }

  for (int r = 0; r < img.domain().size(0) - 1; r++)
    for (int c = 0; c < img.domain().size(1) - 1; c++)
    {
      assert(img(vint2(r, c)) == r * c);
    }


  // Test with border.


  imageNd<int, 2> img2(dims, 1);
  assert(img2.pitch() == sizeof(int) * (dims[1] + 2));
  assert((char*)(&img2(vint2(0,0))) == ((char*)img2.data() + img2.pitch() + sizeof(int)));
  assert((char*)(&img2(vint2(99,199))) == ((char*)&img2(0,0) + 99 * img2.pitch() + 199 * sizeof(int)));
  assert((char*)img2.data_end() == ((char*)(img2.data()) + 102 * img2.pitch()));

  std::vector<int> dim3 = {100, 200, 300};
  imageNd<int, 3> img3(dim3, 1);
  assert(img3.pitch() == sizeof(int) * (dim3[2] + 2));
  assert((char*)(&img3(vint3(0,0,0))) == ((char*)img3.data() + 1 * dim3[1] * img3.pitch() + img3.pitch() + sizeof(int) ));

  //assert(img.data() == )

  int i = 0;
  for (auto& p : img) p = i++;

  auto img_clone = clone(img);
  auto img_clone_border = clone_with_border(img, 3);

  assert(img.domain() == img_clone.domain());
  assert(img.domain() == img_clone_border.domain());

  assert(img.domain() == img_clone_border.domain());

  for (auto p : img.domain())
  {
    assert(img(p) == img_clone(p));
    assert(img(p) == img_clone_border(p));
  }
}
