#include <iostream>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>

int main()
{
  using vpp::image2d;
  using vpp::vint2;
  using vpp::make_box2d;

  // from_opencv, opencv releases memory.
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

  // from_opencv, vpp releases memory.
  {
    image2d<int> v;

    {
      cv::Mat_<int> m(100, 200);
      v = vpp::from_opencv<int>(m);

      assert(v.nrows() == 100);
      assert(v.ncols() == 200);

      m.at<int>(50, 50) = 41;
      assert(*(m.refcount) == 2);
    }

    assert(v(50, 50) == 41);
    for (auto& p : v) p = 42;
    for (auto& p : v) assert(p == 42);
  }

  // To opencv, vpp releases memory.
  {
    image2d<int> v(100, 200);

    {
      cv::Mat_<int> m = vpp::to_opencv(v);

      assert(m.rows == 100);
      assert(m.cols == 200);
      m.at<int>(50, 50) = 41;
    }

    assert(v(50, 50) == 41);
    for (auto& p : v) p = 42;
    for (auto& p : v) assert(p == 42);
  }

  // To opencv, opencv releases memory.
  {
    cv::Mat_<int> m;

    {
      image2d<int> v(100, 200);
      m = vpp::to_opencv(v);

      assert(m.rows == 100);
      assert(m.cols == 200);
      m.at<int>(50, 50) = 41;

      assert(v(50, 50) == 41);

      for (auto& p : v) p = 42;
      for (auto& p : v) assert(p == 42);

    }

    for (auto& p : make_box2d(100, 200)) m.at<int>(p[0], p[1]) = 42;
    for (auto& p : make_box2d(100, 200)) assert(m.at<int>(p[0], p[1]) == 42);
    
    assert(*(m.refcount) == 1);
  }

}
