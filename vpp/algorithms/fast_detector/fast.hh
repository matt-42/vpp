#ifndef VPP_FAST9_DETECTOR_HH_
# define VPP_FAST9_DETECTOR_HH_

# include <vpp/vpp.hh>

namespace vpp
{
  template <typename V>
  std::vector<vint2> fast_detector9(image2d<V>& A,
                                    int th,
                                    const image2d<unsigned char>& mask = image2d<V>(),
                                    std::vector<int>* scores = 0);

  template <typename V>
  std::vector<vint2> fast_detector9_blockwise_maxima(image2d<V>& A,
                                                     int th,
                                                     int block_size,
                                                     const image2d<unsigned char>& mask = image2d<V>(),
                                                     std::vector<int>* scores = 0);

  template <typename V>
  std::vector<vint3> fast_detector9_blockwise_rank(image2d<V>& A,
                                                   int th,
                                                   int block_size,
                                                   int max_point_per_block,
                                                   const image2d<unsigned char>& mask = image2d<V>(),
                                                   std::vector<int>* scores = 0);
  
  template <typename V>
  std::vector<vint2> fast_detector9_local_maxima(image2d<V>& A,
                                                 int th,
                                                 const image2d<unsigned char>& mask = image2d<V>(),
                                                 std::vector<int>* scores = 0);

  template <typename V>
  void fast_detector9_scores(image2d<V>& A,
                             int th, 
                             const std::vector<vint2> keypoints,
                             std::vector<int>& scores);

  template <typename V>
  int fast_detector9_score(image2d<V>& A,
                           int th, 
                           vint2 p);

}

# include <vpp/algorithms/fast_detector/fast.hpp>

#endif
