#include <iostream>
#include <vpp/imageNd.hh>
#include <vpp/opencv_bridge.hh>

int main()
{
  using vpp::image2d;
  using vpp::vint2;

  {
    cv::Mat_<int> m(100, 200);

    {
      image2d<int> v = vpp::from_opencv<int>(cv::Mat(m));

      assert(v.nrows() == 100);
      assert(v.ncols() == 200);

      assert(*(m.refcount) == 2);
    }
    assert(*(m.refcount) == 1);
  }

  {
    image2d<int> v;

    {
      cv::Mat_<int> m(100, 200);
      v = vpp::from_opencv<int>(cv::Mat(m));

      assert(v.nrows() == 100);
      assert(v.ncols() == 200);

      assert(*(m.refcount) == 2);
    }

    for (auto& p : v) p = 42;
    for (auto& p : v) assert(p == 42);
  }


}
