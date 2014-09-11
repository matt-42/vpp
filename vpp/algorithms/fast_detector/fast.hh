#ifndef VPP_FAST9_DETECTOR_HH_
# define VPP_FAST9_DETECTOR_HH_

# include <vpp/vpp.hh>

namespace vpp
{
  template <typename V>
  std::vector<vint2> fast_detector9(image2d<V>& A,
                                    int th, std::vector<int>* scores = 0);

  template <typename V>
  std::vector<vint2> fast_detector9_blockwise_maxima(image2d<V>& A,
                                                     int th,
                                                     int block_size,
                                                     std::vector<int>* scores = 0);

  template <typename V>
  std::vector<vint2> fast_detector9_local_maxima(image2d<V>& A,
                                                 int th,
                                                 std::vector<int>* scores = 0);

}

# include <vpp/algorithms/fast_detector/fast.hpp>

#endif
