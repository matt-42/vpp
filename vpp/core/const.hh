#ifndef VPP_CORE_CONST_HH__
# define VPP_CORE_CONST_HH__

# include <Eigen/Core>

namespace vpp
{
  template <typename T>
  using unconstify = typename std::remove_const<T>::type;
  template <typename T>
  using constify = typename std::add_const<T>::type;
};

#endif
