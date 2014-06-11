#include <iostream>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  imageNd<int, 2> img_test({2,3});


  std::vector<int> dims = {100, 200};


  imageNd<int, 2> img(dims);


  assert(img.domain().size(0) == dims[0]);
  assert(img.domain().size(1) == dims[1]);


  for (int r = 0; r < img.domain().size(0); r++)
    for (int c = 0; c < img.domain().size(1); c++)
    {
      assert(img.coords_to_offset(vint2(r, c)) == img.pitch() * r + c * sizeof(int));
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
  assert(!(long(&img2(0,0)) % 16));
  assert(!(img2.pitch() % 16));
  assert((char*)(&img2(vint2(99,199))) == ((char*)&img2(0,0) + 99 * img2.pitch() + 199 * sizeof(int)));

  std::vector<int> dim3 = {100, 200, 300};
  imageNd<int, 3> img3(dim3, 1);

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

  // Subimage.
  {
    vint2 p1 = vint2(10,10);
    vint2 p2 = vint2(12,15);

    auto sub = img | box2d(p1, p2);
    assert(&sub(0,0) == &img(p1));
    assert(sub.nrows() == (p2[0] - p1[0] + 1));
    assert(sub.ncols() == (p2[1] - p1[1] + 1));
  }

  // Linear interpolation.
  {
    image2d<vuchar1> test(2,2, border(1));

    {
      vuchar1 m; m << 2;
      vint1 a = cast<vint1>(m);
      assert(a[0] == 2);
    }

    {
      vint1 m; m << 2;
      vuchar1 a = cast<vuchar1>(m);
      assert(a[0] == 2);
    }

    test(0,0)[0] = 0;
    test(0,1)[0] = 10;
    test(1,0)[0] = 20;
    test(1,1)[0] = 30;
    int v1 = (10+20+30)/4.f;
    // std::cout << int(test.linear_interpolate(vfloat2(0.5, 0.5))[0]) << std::endl;
    assert(test.linear_interpolate(vfloat2(0.5, 0.5))[0] == v1);
    // int v2 = (0.25 * 0.75) * 10 + (0.25 * 0.75) * 20 + (0.25 * 0.25) * 30;
    // assert(test.linear_interpolate(vfloat2(0.25, 0.25)) == v2);
  }

}
