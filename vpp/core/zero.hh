#ifndef VPP_ZERO_HH__
# define VPP_ZERO_HH__

namespace vpp
{

  template <typename V>
  struct zero
  {
    operator V() { return 0; }
  };

  template <>
  struct zero<float>
  {
    operator float() { return 0.f; }
  };

  template <int N, typename V>
  struct zero<Eigen::Matrix<V, N, 1>>
  {
    operator Eigen::Matrix<V, N, 1>() { return Eigen::Matrix<V, N, 1>::Zero(); }
  };

};

#endif
