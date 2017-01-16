#include <iostream>
#include <vpp/vpp.hh>

using namespace vpp;

template <typename V, typename U>
bool equals(image2d<V>& v, image2d<U>& u)
{
  int eq = true;
  pixel_wise(v, u) | [&] (V& a, U& b) { if (a != b) eq = false; };
  return eq;
}


template <typename V>
void print(image2d<V>& v)
{
  for (int r = 0; r < v.nrows(); r++)
  {
    for (int c = 0; c < v.ncols(); c++)
      std::cout << v(r, c) << " ";
    std::cout << std::endl;
  }
}

int main()
{

  image2d<int> img(4,4);
  vint2 b(2,2);

  auto test_dependency = [&] (int* ref_data, auto dep, int dim)
  {
    image2d<int> ref(img.domain(), _data = (int*)ref_data, _pitch = 4 * sizeof(int));
    int cols[2] = {1,1};
    block_wise(b, img, img, img.domain())(dep)
      | [&] (image2d<int> I, image2d<int> J, box2d d)
    {
      int& cpt = cols[d.p1()[dim] / 2];
      fill(I, cpt);
      cpt++;
    };

    std::cout << "ref: " << std::endl;
    print(ref);
    std::cout << "img: " << std::endl;
    print(img);
    assert(equals(ref, img));
  };
  
  {
    int ref_data[] = {
      1,1,1,1,
      1,1,1,1,
      2,2,2,2,
      2,2,2,2,
    };

    test_dependency(ref_data, _top_to_bottom, 1);
  }

  fill(img, 9);
  {
    int ref_data[] = {
      2,2,2,2,
      2,2,2,2,
      1,1,1,1,
      1,1,1,1,
    };

    test_dependency(ref_data, _bottom_to_top, 1);
  }

  fill(img, 9);
  {
    int ref_data[] = {
      1,1,2,2,
      1,1,2,2,
      1,1,2,2,
      1,1,2,2,
    };

    test_dependency(ref_data, _left_to_right, 0);
  }

  fill(img, 9);
  {
    int ref_data[] = {
      2,2,1,1,
      2,2,1,1,
      2,2,1,1,
      2,2,1,1,
    };

    test_dependency(ref_data, _right_to_left, 0);
  }

  // Check that blocks covers the whole image, and do
  // not overlap with image border.
  {
    image2d<int> img(10,10, _border = 1);
    fill_border_with_value(img, 2);
    fill(img, 0);
    block_wise(vint2(3,3), img) | [] (auto si) { fill(si, 1); };

    print (img);
    for (auto p : img.domain_with_border())
    {
      if (img.has(p))
        assert(img(p) == 1);
      else
        assert(img(p) == 2);
    }
  }
  
}
