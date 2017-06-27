#ifndef VPP_OPENCV_BRIDGE_HH__
# define VPP_OPENCV_BRIDGE_HH__

# include <opencv2/opencv.hpp>
# include <vpp/core/image2d.hh>

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
  OPENCV_TYPEOF_(float, float, 32F);
  OPENCV_TYPEOF_(int, int, 32S);

  struct opencv_data_holder
  {
    int* refcount;
    void* data;
  };

  static void opencv_data_deleter(void* data)
  {
    opencv_data_holder* d = (opencv_data_holder*) data;

    if (d->refcount and *(d->refcount) > 1)
      *(d->refcount) -= 1;
    else if (d->refcount and *(d->refcount) == 1)
      cv::fastFree(d->data);
    delete d;
  }

  template <typename V>
  image2d<V> from_opencv(cv::Mat m)
  {
    if (!m.data) return image2d<V>();

    image2d<V> res(make_box2d(m.rows, m.cols), _data = m.data, _pitch = m.step);

#if (((defined(CV_VERSION_MAJOR) && CV_VERSION_MAJOR == 3)))
    res.set_external_data_holder(new opencv_data_holder{&m.u->refcount, m.data}, opencv_data_deleter);
    #else
    res.set_external_data_holder(new opencv_data_holder{m.refcount, m.data}, opencv_data_deleter);
    #endif
    
    m.addref();
    return res;
  }

  template <typename V>
  cv::Mat to_opencv(image2d<V>& v)
  {
    if (!v.has_data()) return cv::Mat();

    cv::Mat m(v.nrows(), v.ncols(), opencv_typeof<V>::ret, (void*) v.address_of(vint2(0,0)), v.pitch());
    return m;
  }

  template <typename V>
  cv::Mat to_opencv(image2d<V>&& v)
  {
    if (!v.has_data()) return cv::Mat();

    // v is a rvalue (won't survive after the function call)
    // so we need to copy its buffer.
    cv::Mat m(v.nrows(), v.ncols(), opencv_typeof<V>::ret);

    image2d<V> out = from_opencv<V>(m);
    copy(v, out);

    return m;
  }

};

#endif
