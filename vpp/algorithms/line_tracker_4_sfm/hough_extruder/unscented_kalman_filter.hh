#pragma once

#include "Eigen/Dense"
#include <vpp/vpp.hh>
#include <vector>
#include <string>
#include <fstream>

using Eigen::MatrixXd;
using Eigen::VectorXd;
using namespace vpp;

class unscented_kalman_filter {
public:
  bool is_initialized_;

  int state_dimension;

  int augmented_state_dimension;

  int measurement_dimension;

  double sigma_point_spreading_parameter;

  VectorXd state_vector;

  MatrixXd covariance_matrix;

  MatrixXd predicted_sigmas_matrix;

  long long previous_timestamp_;

  double standard_deviation_longit_acc;

  double standard_deviation_yaw;

  double standard_deviation_noise1;

  double standard_deviation_noise2;


  VectorXd weights_;

  /**
   * Constructor
   */
  unscented_kalman_filter();

  /**
   * Destructor
   */
  virtual ~unscented_kalman_filter();

  void add_new_dectection(const vint2 values, float dt);

  void initialize_the_track(const vint2 values, float dt);

  void only_update_track(float dt);

  void augmented_sigma_points(MatrixXd &Xsig_out);

  void sigma_point_prediction(const MatrixXd  &Xsig_aug, const double delta_t, MatrixXd  &Xsig_out);

  void predict_mean_and_covariance(const MatrixXd &Xsig_pred, VectorXd &x_out, MatrixXd &P_out);

  void predict_measurement(VectorXd &z_out, MatrixXd &S_out, MatrixXd &Tc_out);

  void prediction(double delta_t);

  void update(vint2 values,float dt, VectorXd &z_pred, MatrixXd &Tc, MatrixXd &S);

};

#include "unscented_kalman_filter.hpp"


