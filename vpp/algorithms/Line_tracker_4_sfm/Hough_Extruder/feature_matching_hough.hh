#ifndef FEATURE_MATCHING_HOUGH_HH
#define FEATURE_MATCHING_HOUGH_HH

#include <vpp/core/keypoint_trajectory.hh>
#include <vpp/core/keypoint_container.hh>
#include <vpp/core/symbols.hh>
#include <vpp/algorithms/descriptor_matcher.hh>
#include "track.hh"
#include "vpp/algorithms/Line_tracker_4_sfm/miscellanous/define.hh"


namespace vpp {

float val_diff = 0;
int nb_diff = 0;

struct feature_matching_hough_ctx {
    feature_matching_hough_ctx(box2d domain) : keypoints(domain), frame_id(0) {}

    // Keypoint container.
    // ctx.keypoint[i] to access the ith keypoint.
    keypoint_container<keypoint<int>, int> keypoints;

    // Trajectory container.
    // ctx.trajectory[i].position_at_frame(j) to access the ith keypoint position at frame j.
    std::vector<keypoint_trajectory> trajectories;

    std::vector<track> list_track;



    // Current frame id.
    int frame_id;
};



feature_matching_hough_ctx feature_matching_hough_init(box2d domain);

template <typename... OPTS>
void feature_matching_hough_update_three_first(feature_matching_hough_ctx& ftx,
                                               std::vector<float>& frame1,
                                               std::vector<float>& frame2,
                                               image2d<uchar> &frame_gradient,image2d<uchar> &frame_point,
                                               int type_video,image2d<uchar> img,
                                               int T_theta,int rhomax,bool first, Type_output type_sortie,
                                               int &id_trackers,Type_Lines type_line,Frequence freq,std::vector<vint2>& old_vects,
                                               int wkf,
                                               OPTS... options);

template <typename... OPTS>
void feature_matching_hough_update_three_first_3_frames(feature_matching_hough_ctx& ftx,
                                                        std::vector<float>& frame1,
                                                        std::vector<float>& frame2,
                                                        std::vector<float>& frame3,
                                                        int nb_matcher,
                                                        image2d<uchar> &frame_grad,
                                                        image2d<uchar> &frame_point,
                                                        std::vector<vint2>& new_values1,
                                                        int type_video,
                                                        image2d<uchar> img,
                                                        int T_theta,int rhomax,bool first,
                                                        Type_output type_sortie,int &id_trackers,
                                                        Type_Lines type_line,Frequence freq,std::list<vint2>& old_vects,
                                                        //float precision_runtime_balance = 0,
                                                        OPTS... options);

template <typename... OPTS>
void feature_matching_hough_update_three_first_mat(feature_matching_hough_ctx& ftx,
                                               std::vector<float>& frame1,
                                               std::vector<float>& frame2,
                                               Mat &frame_grad,Sclare_rho scale,
                                               int type_video,image2d<uchar> img, Mat &img_ori,
                                               int T_theta,int rhomax,bool first,
                                               Type_output type_sortie,int &id_trackers,
                                               Type_Lines type_line,Frequence freq,std::list<vint2>& old_vects,
                                               //float precision_runtime_balance = 0,
                                               OPTS... options);

template <typename... OPTS>
void feature_matching_hough_update_local(feature_matching_hough_ctx& ftx,
                                    std::vector<float>& frame1,
                                    std::vector<float>& frame2,int type_video,image2d<vuchar1> img,
                                   int T_theta,int rhomax,bool first, std::vector<vint2>& old_values,
                                   int &id_trackers,
                                   //float precision_runtime_balance = 0,
                                   OPTS... options);

float Distance_between_curve_L1(std::vector<float> frame1, std::vector<float>  frame2, int taille);

float Distance_between_curve_L2(std::vector<float> frame1, std::vector<float>  frame2,  int taille_theta, int taille_rho);

float sum_vector(std::vector<float> a);

float mean_vector(std::vector<float> a);

float sqsum(std::vector<float> a);

float stdev(std::vector<float> nums);

std::vector<float> operator-(std::vector<float> a, float b);

std::vector<float> operator*(std::vector<float> a, std::vector<float> b);

float pearsoncoeff(std::vector<float> X, std::vector<float> Y);

float Correlation_Matrix_Pearson(Eigen::MatrixXd M1,Eigen::MatrixXd M2,int taille);

std::vector<int> getDuplicata(feature_matching_hough_ctx ftx);

inline
float computeDistanceHoughSpace(float rhomax,float rho_p,float theta_p,float rho_q,float theta_q);


void brute_force_matching(std::vector<vint2> descriptor1, std::vector<vint2> descriptor2,std::vector<int>& matches,int type);
}

#include "feature_matching_hough.hpp"

#endif // FEATURE_MATCHING_HOUGH_HH
