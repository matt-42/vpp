#ifndef VPP_FAST9_DETECTOR_HH_
# define VPP_FAST9_DETECTOR_HH_

# include <vpp/vpp.hh>

# include <vpp/algorithms/symbols.hh>

namespace vpp
{

  // FAST9 detector.

  // Options :
  //  _local_maxima: Local maxima keypoint selection.
  //  _blockwise: blockwise keypoint selection (one kp per block)
  //  _blockwise_rank: blockwise rank selection (several kp per block).
  //
  //  _block_size = int(): block size ised by _blockwise and _blockwise_rank.
  //  _max_points_per_block = int(): block size used by _blockwise and _blockwise_rank.
  //  _mask = image2d<unsigned char>
  //  _scores = &std::vector<int>
  //
  // example: fast_detector9(image, th, _local_maxima, _mask = mask_image);
  //
  template <typename V, typename... OPTS>
  std::vector<vint2> fast9(const image2d<V>& A,
                           int th,
                           OPTS... opts);

  template <typename V, typename KPS>
  void fast9_scores(const image2d<V>& A,
                    int th, 
                    const KPS& keypoints,
                    std::vector<int>& scores);

  template <typename V>
  int fast9_score(const image2d<V>& A,
                  int th, 
                  vint2 p);
  
  // Old API only used internally.

  template <typename V>
  std::vector<vint2> fast_detector9(const image2d<V>& A,
                                    int th,
                                    const image2d<unsigned char>& mask,
                                    std::vector<int>* scores = 0);

  template <typename V>
  std::vector<vint2> fast_detector9_blockwise_maxima(const image2d<V>& A,
                                                     int th,
                                                     int block_size,
                                                     const image2d<unsigned char>& mask,
                                                     std::vector<int>* scores = 0);

  template <typename V>
  std::vector<vint3> fast_detector9_blockwise_rank(const image2d<V>& A,
                                                   int th,
                                                   int block_size,
                                                   int max_point_per_block,
                                                   const image2d<unsigned char>& mask,
                                                   std::vector<int>* scores = 0);
  
  template <typename V>
  std::vector<vint2> fast_detector9_local_maxima(const image2d<V>& A,
                                                 int th,
                                                 const image2d<unsigned char>& mask,
                                                 std::vector<int>* scores = 0);

}

# include <vpp/algorithms/fast_detector/fast.hpp>

#endif
