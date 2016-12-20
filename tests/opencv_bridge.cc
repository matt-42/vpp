#include <iostream>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>

void check_cv_refcount(const cv::Mat& m, int i)
{
#if (((defined(CV_VERSION_MAJOR) && CV_VERSION_MAJOR == 3)))
      assert((m.u->refcount) == i);
#else
      assert(*(m.refcount) == i);      
#endif

}

int main()
{
  using vpp::image2d;
  using vpp::vint2;
  using vpp::make_box2d;

  // from_opencv, opencv releases memory.
  {
    cv::Mat_<int> m(100, 200);

    {
      image2d<int> v = vpp::from_opencv<int>(m);

      assert(v.nrows() == 100);
      assert(v.ncols() == 200);

      check_cv_refcount(m, 2);

    }
    check_cv_refcount(m, 1);
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

      check_cv_refcount(m, 2);
    }

    assert(v(50, 50) == 41);
    for (auto& p : v) p = 42;
    for (auto& p : v) assert(p == 42);
  }

  // To opencv, vpp releases memory.
  {
    image2d<int> v(100, 200);

    {
      cv::Mat m = vpp::to_opencv(v);

      assert(m.rows == 100);
      assert(m.cols == 200);

      v(50, 50) = 0;;
      m.at<int>(50, 50) = 41;
      assert(v(50, 50) == 41);
    }

    v(50, 51) = 42;
    assert(v(50, 50) == 41);
    assert(v(50, 51) == 42);

    // for (auto& p : v) p = 42;
    // for (auto& p : v) assert(p == 42);
  }

  // To opencv, opencv releases memory.
  // Does not work because cv::fastFree cannot release
  // a buffer given by malloc.
  // {
  //   cv::Mat_<int> m;

  //   {
  //     image2d<int> v(100, 200);
  //     m = vpp::to_opencv(v);

  //     assert(m.rows == 100);
  //     assert(m.cols == 200);
  //     m.at<int>(50, 50) = 41;

  //     assert(v(50, 50) == 41);

  //     for (auto& p : v) p = 42;
  //     for (auto& p : v) assert(p == 42);

  //   }

  //   m.at<int>(10,10) = 42;
  //   assert(m.at<int>(10,10) == 42);
  //   // for (auto& p : make_box2d(100, 200)) m.at<int>(p[0], p[1]) = 42;
  //   // for (auto& p : make_box2d(100, 200)) assert(m.at<int>(p[0], p[1]) == 42);
    
  //   assert(*(m.refcount) == 1);
  // }

}
