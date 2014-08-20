#ifndef VPP_SLAM_TRIANGULATE_HH_
# define VPP_SLAM_TRIANGULATE_HH_

# include <Eigen/Core>
# include <Eigen/SVD>

namespace vpp
{

  // Triangulate a 3d point given 2 projections.
  inline vfloat3 triangulate(vfloat3 x,
                             vfloat3 y,
                             Eigen::Matrix4f A,
                             Eigen::Matrix4f B)
  {
    using namespace Eigen;

    MatrixXf S(6, 4);

    S.block<1, 4>(0,0) = x[1] * A.block<1, 4>(2,0) - x[2] * A.block<1, 4>(1,0);
    S.block<1, 4>(1,0) = x[2] * A.block<1, 4>(0,0) - x[0] * A.block<1, 4>(2,0);
    S.block<1, 4>(2,0) = x[0] * A.block<1, 4>(1,0) - x[1] * A.block<1, 4>(0,0);
    S.block<1, 4>(3,0) = y[1] * B.block<1, 4>(2,0) - y[2] * B.block<1, 4>(1,0);
    S.block<1, 4>(4,0) = y[2] * B.block<1, 4>(0,0) - y[0] * B.block<1, 4>(2,0);
    S.block<1, 4>(5,0) = y[0] * B.block<1, 4>(1,0) - y[1] * B.block<1, 4>(0,0);

    JacobiSVD<MatrixXf> svd(S, ComputeFullV);
    return (svd.matrixV().block<3, 1>(0, 3) / svd.matrixV()(3, 3));
  }

};

#endif
