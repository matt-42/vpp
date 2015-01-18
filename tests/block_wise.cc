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
  int i = 0;
  // block_wise(b, img, img, img.domain())//(_col_backward)
  //   | [&] (image2d<int> I, image2d<int> J, box2d d)
  // {
  //   assert(I.nrows() == b[0]);
  //   assert(I.ncols() == b[1]);
  //   fill(I, i);
  //   i++;
  // };

  // pixel_wise(img.domain(), img) | [&] (vint2 c, int& v)
  // {
  //   c[0] /= b[0];
  //   c[1] /= b[1];
  //   int idx = (b[1] * c[0] + c[1]);
  //   assert(idx == v);
  // };

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

    test_dependency(ref_data, _col_forward, 1);
  }

  fill(img, 9);
  {
    int ref_data[] = {
      2,2,2,2,
      2,2,2,2,
      1,1,1,1,
      1,1,1,1,
    };

    test_dependency(ref_data, _col_backward, 1);
  }

  fill(img, 9);
  {
    int ref_data[] = {
      1,1,2,2,
      1,1,2,2,
      1,1,2,2,
      1,1,2,2,
    };

    test_dependency(ref_data, _row_forward, 0);
  }

  fill(img, 9);
  {
    int ref_data[] = {
      2,2,1,1,
      2,2,1,1,
      2,2,1,1,
      2,2,1,1,
    };

    test_dependency(ref_data, _row_backward, 0);
  }
  
    // image2d<int> ref(img.domain(), _data = (int*)ref_data, _pitch = 4 * sizeof(int));
    // int cols[2] = {1,1};
    // block_wise(b, img, img, img.domain())(_col_forward)
    //   | [&] (image2d<int> I, image2d<int> J, box2d d)
    // {
    //   int& cpt = cols[d.p1()[1] / 2];
    //   fill(I, cpt);
    //   cpt++;
    // };

    // std::cout << ref(0,0) << " " << img(0,0) << std::endl;
    // assert(equals(ref, img));
  // }
    // ref

  // {
  //   image2d<int> img(4,4);

  // }
}
