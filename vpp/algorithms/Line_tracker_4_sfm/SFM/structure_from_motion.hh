#ifndef STRUCTURE_FROM_MOTION_HH
#define STRUCTURE_FROM_MOTION_HH


#include <ctime>
#include <cstdlib>

#include <vpp/vpp.hh>
#include "algorithms/Line_tracker_4_sfm/miscellanous/operations.hh"

#include <Eigen/Core>
#include <Eigen/SVD>
#include <unsupported/Eigen/Polynomials>


using namespace vpp;
using namespace std;
using namespace Eigen;
using namespace iod;
using namespace cv;

namespace vpp
{

//from http://www.mip.informatik.uni-kiel.de/tiki-index.php?page=Lilian+Zhang

struct structure_from_motion_ctx {
    structure_from_motion_ctx(box2d domain) : keypoints(domain) {}

    // Keypoint container.
    // ctx.keypoint[i] to access the ith keypoint.
    keypoint_container<keypoint<int>, int> keypoints;

    std::vector<vfloat3> start_points;
    std::vector<vfloat3> end_points;
    std::vector<vfloat3> directions;
};

structure_from_motion_ctx structure_from_motion_init(box2d domain);

void initialize_rotation_matrix();
void initialize_translation_matrix();
void Pose_Estimation_From_Line_Correspondence(MatrixXf start_points, MatrixXf end_points,
                                              MatrixXf directions, MatrixXf points,
                                              MatrixXf &rot_cw, VectorXf &pos_cw);
inline
void calcampose(MatrixXf XXc, MatrixXf XXw, int n, MatrixXf &R2, VectorXf &t2);
inline
void R_and_T(MatrixXf &rot_cw, VectorXf &pos_cw,MatrixXf start_points, MatrixXf end_points,
             MatrixXf P1w,MatrixXf P2w,MatrixXf initRot_cw,VectorXf initPos_cw,
             int maxIterNum,float TerminateTh,int nargin);

inline
vfloat3 xcross(vfloat3 a,vfloat3 b);


void triangulation();
void bundle_adjustement();


}

#include "structure_from_motion.hpp"

#endif // STRUCTURE_FROM_MOTION_HH
