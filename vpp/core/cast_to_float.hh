#ifndef VPP_CAST_TO_FLOAT_HH__
# define VPP_CAST_TO_FLOAT_HH__

# include <Eigen/Core>

namespace vpp
{

  template <typename V>
  struct cast_to_float_
  {
    typedef float ret;
  };

  template <int N, typename V>
  struct cast_to_float_<Eigen::Matrix<V, N, 1>>
  {
    typedef Eigen::Matrix<float, N, 1> ret;
  };

  template <typename V>
  using cast_to_float = typename cast_to_float_<V>::ret;
};

#endif
