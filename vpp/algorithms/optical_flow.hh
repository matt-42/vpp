#pragma once

namespace vpp
{

  // Compute a semi dense flow field with the following assumptions :
  //   - The keypoint set should be dense enough to let the estimation rely on spatial regularity.
  //   - The nscales and winsize parameters should be large enough to recover the motion.
  //
  // Parameters:
  //   nscales = 3: The number of scales used by multiscales approaches.
  //   winsize = 9: The window size support for SSD computation
  //   patchsize = 3: Suppose that every NxN cell contains the same motion.
  //   propagation = 2: The number of propagation iterations.
  //   min_scale = 1: Minimum scale used to estimate the flow.
  //
  // Todo parameters:
  //   _epipolar_flow: Guide the motion estimation with the epipolar lines.
  //   _epipolar_line_filter: Activate the epipolar line filter.
  //   _fundamental_matrix: Fundamental matrix.
  //   _qr_ratio = 0-1: From 0 for maximum speed to 1 for maximum
  //                   quality.  The best appropriate optical flow
  //                   method will be selected depending of this
  //                   parameter. Note : evaluation is done on public
  //                   optical flow datasets (KITTI, middlebury, ...).
  //
  //   _regularisation ?
  //
  template <typename K, typename F, typename... OPTS>
  inline void
  semi_dense_optical_flow(const K& keypoints,
                          F match_callback,
                          const image2d<unsigned char>& i1,
                          const image2d<unsigned char>& i2,
                          OPTS... options);

  
  // Todo.
  
  // Compute the dense optical flow from two images.
  // Parameters:
  //   semi_dense_optical_flow parameters
  //   mask: if (mask(p)) compute the flow
  // Todo.
  // template <typename F, typename... OPTS>
  // inline void
  // dense_optical_flow(F flow_callback,
  //                    const image2d<unsigned char>& i1,
  //                    const image2d<unsigned char>& i2,
  //                    OPTS... options);

  // Computes the flow vectors of a sparse set of keypoints. This
  // relies on detecting FAST keypoints in \i1 and \i2 and matching
  // them with the set of keypoint.
  //
  // Parameters:
  //   search_radius = 100
  //   winsize = 11
  template <typename K, typename F, typename... OPTS>
  inline void
  sparse_optical_flow(const K& keypoints,
                      F match_callback,
                      const image2d<unsigned char>& i1,
                      const image2d<unsigned char>& i2,
                      OPTS... options);

  template <typename K, typename F, typename... OPTS>
  inline void
  sparse_optical_flow(const image2d<unsigned char>& i1,
                      const image2d<unsigned char>& i2,
                      OPTS... options);
  
}

#include <vpp/algorithms/optical_flow/semi_dense_optical_flow.hpp>
#include <vpp/algorithms/optical_flow/dense_optical_flow.hpp>
#include <vpp/algorithms/optical_flow/sparse_optical_flow.hh>
