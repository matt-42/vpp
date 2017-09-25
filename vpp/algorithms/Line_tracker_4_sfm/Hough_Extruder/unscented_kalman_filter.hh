#ifndef UKF_HH
#define UKF_HH

#include "Eigen/Dense"
#include <vpp/vpp.hh>
#include <vector>
#include <string>
#include <fstream>

using Eigen::MatrixXd;
using Eigen::VectorXd;
using namespace vpp;

class Unscented_Kalman_Filter {
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
  Unscented_Kalman_Filter();

  /**
   * Destructor
   */
  virtual ~Unscented_Kalman_Filter();

  void AddNewDectection(const vint2 values, float dt);

  void InitializeTheTrack(const vint2 values, float dt);

  void onlyUpdateTrack(float dt);

  void AugmentedSigmaPoints(MatrixXd &Xsig_out);

  void SigmaPointPrediction(const MatrixXd  &Xsig_aug, const double delta_t, MatrixXd  &Xsig_out);

  void PredictMeanAndCovariance(const MatrixXd &Xsig_pred, VectorXd &x_out, MatrixXd &P_out);

  void PredictMeasurement(VectorXd &z_out, MatrixXd &S_out, MatrixXd &Tc_out);

  void Prediction(double delta_t);

  void Update(vint2 values,float dt, VectorXd &z_pred, MatrixXd &Tc, MatrixXd &S);

};

#include "unscented_kalman_filter.hpp"

#endif // UKF_HH
