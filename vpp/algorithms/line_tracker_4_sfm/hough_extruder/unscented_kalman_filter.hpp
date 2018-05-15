


#include "unscented_kalman_filter.hh"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 */
unscented_kalman_filter::unscented_kalman_filter() {
    // Set initialization to false initially
    is_initialized_  = false;

    ///* State dimension
    state_dimension = 5;

    ///* Augmented state dimension
    augmented_state_dimension = 7;

    //* radar measurement dimension
    measurement_dimension = 2;

    ///* Sigma point spreading parameter
    sigma_point_spreading_parameter = 3 - augmented_state_dimension;

    // initial state vector
    state_vector = VectorXd(5);

    // initial covariance matrix
    covariance_matrix = MatrixXd::Zero(5, 5);

    // initialize the sigma points matrix with zeroes
    predicted_sigmas_matrix = MatrixXd::Zero(state_dimension, 2 * augmented_state_dimension + 1);

    ///* time when the state is true, in us
    previous_timestamp_ = 0;

    // Process noise standard deviation longitudinal acceleration in m/s^2
    standard_deviation_longit_acc = 3;

    // Process noise standard deviation yaw acceleration in rad/s^2
    standard_deviation_yaw = 0.7;

    // Laser measurement noise standard deviation position1 in m
    standard_deviation_noise1 = 0.15;

    // Laser measurement noise standard deviation position2 in m
    standard_deviation_noise2 = 0.15;


    ///* Weights of sigma points
    weights_ = VectorXd::Zero(2*augmented_state_dimension+1);


    // set weights
    double weight_0 = sigma_point_spreading_parameter/(sigma_point_spreading_parameter+augmented_state_dimension);
    weights_(0) = weight_0;
    for (int i=1; i<2*augmented_state_dimension+1; i++) {  //2n+1 weights
        double weight = 0.5/(augmented_state_dimension+sigma_point_spreading_parameter);
        weights_(i) = weight;
    }

    return;

}

unscented_kalman_filter::~unscented_kalman_filter() {}



void unscented_kalman_filter::add_new_dectection(const vint2 values, float dt) {

    cout << "ajouter un nouveau point " << endl;
    if (!is_initialized_) {
        //  Initialize the state x_,state covariance matrix P_
        initialize_the_track(values,dt);
        //previous_timestamp_ = meas_package.timestamp_;
        is_initialized_ = true;
        return;
    }
    double delta_t =  dt;
    prediction(delta_t);
    //mean predicted measurement
    VectorXd z_pred = VectorXd::Zero(measurement_dimension);
    //measurement covariance matrix S
    MatrixXd S = MatrixXd::Zero(measurement_dimension,measurement_dimension);
    // cross-correlation matrix Tc
    MatrixXd Tc = MatrixXd::Zero(state_dimension, measurement_dimension);
    // get predictions for x,S and Tc in Lidar space
    predict_measurement(z_pred, S, Tc);
    cout << "la prediction " << z_pred[0] << "  " << z_pred[1] << endl;
    // update the state using the LIDAR measurement
    update(values,dt, z_pred, Tc, S);
    cout << "la vraie valeur " << values[0] << "  " << values[1] << endl;
    // update the time
    return;
}

void unscented_kalman_filter::only_update_track(float dt)
{
    double delta_t =  dt;
    prediction(delta_t);
    //mean predicted measurement
    VectorXd z_pred = VectorXd::Zero(measurement_dimension);
    //measurement covariance matrix S
    MatrixXd S = MatrixXd::Zero(measurement_dimension,measurement_dimension);
    // cross-correlation matrix Tc
    MatrixXd Tc = MatrixXd::Zero(state_dimension, measurement_dimension);
    // get predictions for x,S and Tc in Lidar space
    predict_measurement(z_pred, S, Tc);
    cout << "la prediction " << z_pred[0] << "  " << z_pred[1] << endl;
    return;
}

void unscented_kalman_filter::initialize_the_track(const vint2 values,float dt) {
    cout << "initialisation du filtre " << endl;
    // initialize state covariance matrix P
    covariance_matrix = MatrixXd(5, 5);
    covariance_matrix <<   1, 0,  0,  0,  0,
            0,  1, 0,  0,  0,
            0,  0,  1, 0,  0,
            0,  0,  0,  1, 0,
            0,  0,  0,  0,  1;
    // Initialize state.
    state_vector << values[0],values[1],0,0,0;
    return;
}

/**
 * @param MatrixXd &Xsig_out. Computes augmented sigma points in state space
 */
void unscented_kalman_filter::augmented_sigma_points(MatrixXd &Xsig_out){
    //create augmented mean vector
    static VectorXd x_aug = VectorXd(augmented_state_dimension);
    //create augmented state covariance
    static MatrixXd P_aug = MatrixXd(augmented_state_dimension, augmented_state_dimension);
    //create sigma point matrix
    static MatrixXd Xsig_aug = MatrixXd(augmented_state_dimension, 2 * augmented_state_dimension + 1);
    //create augmented mean state
    x_aug.head(5) = state_vector;
    x_aug(5) = 0;
    x_aug(6) = 0;
    //create augmented covariance matrix
    P_aug.fill(0.0);
    P_aug.topLeftCorner(5,5) = covariance_matrix;
    P_aug(5,5) = pow(standard_deviation_longit_acc,2);
    P_aug(6,6) = pow(standard_deviation_yaw,2);
    //create square root matrix
    MatrixXd L = P_aug.llt().matrixL();
    //create augmented sigma points
    Xsig_aug.col(0)  = x_aug;
    for (int i = 0; i< augmented_state_dimension; i++)
    {
        Xsig_aug.col(i+1)       = x_aug + sqrt(sigma_point_spreading_parameter+augmented_state_dimension) * L.col(i);
        Xsig_aug.col(i+1+augmented_state_dimension) = x_aug - sqrt(sigma_point_spreading_parameter+augmented_state_dimension) * L.col(i);
    }
    Xsig_out = Xsig_aug;
    return;
}

/**
 * @param MatrixXd &Xsig_out. predicts augmented sigma points
 */
void unscented_kalman_filter::sigma_point_prediction(const MatrixXd &Xsig_aug, const double delta_t, MatrixXd  &Xsig_out){
    //create matrix with predicted sigma points as columns
    static MatrixXd Xsig_pred = MatrixXd(state_dimension, 2 * augmented_state_dimension + 1);
    //predict sigma points
    for (int i = 0; i< 2*augmented_state_dimension+1; i++)
    {
        //extract values for better readability
        double p_x = Xsig_aug(0,i);
        double p_y = Xsig_aug(1,i);
        double v = Xsig_aug(2,i);
        double yaw = Xsig_aug(3,i);
        double yawd = Xsig_aug(4,i);
        double nu_a = Xsig_aug(5,i);
        double nu_yawdd = Xsig_aug(6,i);
        //predicted state values
        double px_p, py_p;
        //avoid division by zero
        if (fabs(yawd) > 0.001) {
            px_p = p_x + v/yawd * ( sin (yaw + yawd*delta_t) - sin(yaw));
            py_p = p_y + v/yawd * ( cos(yaw) - cos(yaw+yawd*delta_t) );
        }
        else {
            px_p = p_x + v*delta_t*cos(yaw);
            py_p = p_y + v*delta_t*sin(yaw);
        }
        double v_p = v;
        double yaw_p = yaw + yawd*delta_t;
        double yawd_p = yawd;
        //add noise
        px_p = px_p + 0.5*nu_a*delta_t*delta_t * cos(yaw);
        py_p = py_p + 0.5*nu_a*delta_t*delta_t * sin(yaw);
        v_p = v_p + nu_a*delta_t;

        yaw_p = yaw_p + 0.5*nu_yawdd*delta_t*delta_t;
        yawd_p = yawd_p + nu_yawdd*delta_t;

        //write predicted sigma point into right column
        Xsig_pred(0,i) = px_p;
        Xsig_pred(1,i) = py_p;
        Xsig_pred(2,i) = v_p;
        Xsig_pred(3,i) = yaw_p;
        Xsig_pred(4,i) = yawd_p;
    }

    Xsig_out = Xsig_pred;

    return;

}


void unscented_kalman_filter::predict_mean_and_covariance(const MatrixXd &Xsig_pred, VectorXd &x_out, MatrixXd &P_out) {

    //create vector for predicted state
    static VectorXd x = VectorXd(state_dimension);

    //create covariance matrix for prediction
    static MatrixXd P = MatrixXd(state_dimension, state_dimension);

    //predicted state mean
    x.fill(0.0);

    for (int i = 0; i < 2 * augmented_state_dimension + 1; i++) {  //iterate over sigma points
        x = x + weights_(i) * Xsig_pred.col(i);
    }


    //predicted state covariance matrix
    P.fill(0.0);
    for (int i = 0; i < 2 * augmented_state_dimension + 1; i++) {  //iterate over sigma points

        // state difference
        VectorXd x_diff = Xsig_pred.col(i) - x;
        //angle normalization
        if (x_diff(3)> M_PI)
        {
            //cout << "normalisation " << x_diff(3) << endl;
            x_diff(3) = atan2(sin(x_diff(3)),cos(x_diff(3)));
        }

        P = P + weights_(i) * x_diff * x_diff.transpose() ;

    }

    //print result
    //std::cout << "Predicted state px,py,v,psi,psi_dot" << std::endl;
    //std::cout << x << std::endl;
    //std::cout << "Predicted covariance matrix P " << std::endl;
    //std::cout << P << std::endl;

    //write result
    x_out = x;
    P_out = P;

    return;
}


void unscented_kalman_filter::predict_measurement(VectorXd &z_out, MatrixXd &S_out, MatrixXd &Tc_out) {

    //create matrix for sigma points in measurement space
    static MatrixXd Zsig = MatrixXd(measurement_dimension, 2 * augmented_state_dimension + 1);

    //transform sigma points into measurement space
    for (int i = 0; i < 2 * augmented_state_dimension + 1; i++) {  //2n+1 sigma points

        // measurement model
        Zsig(0,i) = predicted_sigmas_matrix(0,i);          //px
        Zsig(1,i) = predicted_sigmas_matrix(1,i);          //py

    }

    //mean predicted measurement
    static VectorXd z_pred = VectorXd(measurement_dimension);
    z_pred.fill(0.0);
    for (int i=0; i < 2*augmented_state_dimension+1; i++) {
        z_pred = z_pred + weights_(i) * Zsig.col(i);
    }

    //measurement covariance matrix S
    static MatrixXd S = MatrixXd(measurement_dimension,measurement_dimension);
    S.fill(0.0);
    //create matrix for cross correlation Tc
    static MatrixXd Tc = MatrixXd(state_dimension, measurement_dimension);
    Tc.fill(0.0);

    for (int i = 0; i < 2 * augmented_state_dimension + 1; i++) {  //2n+1 simga points
        //residual
        VectorXd z_diff = Zsig.col(i) - z_pred;

        S = S + weights_(i) * z_diff * z_diff.transpose();

        // state difference
        VectorXd x_diff = predicted_sigmas_matrix.col(i) - state_vector;

        Tc = Tc + weights_(i) * x_diff * z_diff.transpose();

    }

    //add measurement noise covariance matrix
    static MatrixXd R = MatrixXd(measurement_dimension,measurement_dimension);
    R <<    pow(standard_deviation_noise1,2), 0,
            0, pow(standard_deviation_noise2,2);
    S = S + R;

    //write result
    z_out = z_pred;
    S_out = S;
    Tc_out = Tc;

    return;

}


void unscented_kalman_filter::prediction(double delta_t) {

    static MatrixXd Xsig_aug = MatrixXd(2*augmented_state_dimension + 1, augmented_state_dimension);

    //create example matrix with predicted sigma points
    static MatrixXd Xsig_pred = MatrixXd(state_dimension, 2 * augmented_state_dimension + 1);

    // compute augmented sigma points
    augmented_sigma_points(Xsig_aug);

    // predict augmented sigma points
    //insert timer
    sigma_point_prediction(Xsig_aug, delta_t, Xsig_pred);

    static VectorXd x_pred = VectorXd(state_dimension);
    static MatrixXd P_pred = MatrixXd(state_dimension, state_dimension);

    predict_mean_and_covariance(Xsig_pred,x_pred, P_pred);

    state_vector = x_pred;
    covariance_matrix = P_pred;
    predicted_sigmas_matrix = Xsig_pred;

    return;

}



void unscented_kalman_filter::update(vint2 values, float dt, VectorXd &z_pred, MatrixXd &Tc, MatrixXd &S) {

    //mean predicted measurement
    static VectorXd z = VectorXd::Zero(measurement_dimension);
    z << values[0],values[1];

    //Kalman gain K;
    static MatrixXd K = MatrixXd::Zero(state_dimension,measurement_dimension);
    K = Tc * S.inverse();

    //residual
    static VectorXd z_diff = VectorXd::Zero(measurement_dimension);
    z_diff = z - z_pred;

    //update state mean and covariance matrix
    state_vector = state_vector + K * z_diff;
    covariance_matrix = covariance_matrix - K*S*K.transpose();
    return;

}


