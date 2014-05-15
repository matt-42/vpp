#ifndef VPP_OPENCV_BRIDGE_HH__
# define VPP_OPENCV_BRIDGE_HH__

# include <opencv2/opencv.hpp>
# include <vpp/image2d.hh>

namespace vpp
{

  template <typename T>
  struct opencv_typeof;

#define OPENCV_TYPEOF(T, V)			\
  template <>					\
  struct opencv_typeof<T>                       \
  {						\
    enum { ret = V };				\
  };

#define OPENCV_TYPEOF_(BT, T, CV)		\
  OPENCV_TYPEOF(BT, CV_##CV##C1);		\
  OPENCV_TYPEOF(v##T##1, CV_##CV##C1);		\
  OPENCV_TYPEOF(v##T##2, CV_##CV##C2);		\
  OPENCV_TYPEOF(v##T##3, CV_##CV##C3);		\
  OPENCV_TYPEOF(v##T##4, CV_##CV##C4);

  OPENCV_TYPEOF_(unsigned char, uchar, 8U);
  OPENCV_TYPEOF_(char, char, 8S);
  OPENCV_TYPEOF_(unsigned short, ushort, 16U);
  OPENCV_TYPEOF_(short, short, 16S);
  OPENCV_TYPEOF_(float, float, 32S);
  OPENCV_TYPEOF_(int, int, 32S);

  // template <typename V>
  // image2d<V> from_opencv(cv::Mat m)
  // {
  //   int dims[] = { m.rows, m.cols };
  //   m.addref();
  //   return image2d<V>(dims, 0, (V*) m.data, m.step, true);
  // }

  template <typename V>
  image2d<V> from_opencv(cv::Mat&& m)
  {
    image2d<V> res({ m.rows, m.cols }, 0, (V*) m.data, m.step, false);
    res.set_external_refcount(m.refcount);
    m.addref();
    return res;
  }

  template <typename V>
  cv::Mat to_opencv(image2d<V>& m)
  {
    return cv::Mat(m.nrows(), m.ncols(), opencv_typeof<V>::ret, (void*) m.address_of(vint2(0,0)), m.pitch());
  }

};

#endif
