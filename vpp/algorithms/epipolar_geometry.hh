#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

namespace vpp
{

  // http://www.cse.psu.edu/~rcollins/CSE486/lecture19_6pp.pdf
  // Page 5.
  inline vfloat2 epipole_left(const Eigen::Matrix3f& F)
  {
    // F e_l = 0.
    Eigen::EigenSolver<Eigen::Matrix3d> es(F.cast<double>().transpose() * F.cast<double>());

    auto values = es.eigenvalues();
    auto vectors = es.eigenvectors();
    vdouble2 epipole;
    double min_ev = FLT_MAX;
    for (int i = 0; i < values.size(); i++)
      if (values[i].real() < min_ev)
      {
        min_ev = values[i].real();
        auto vc = vectors.col(i);
        epipole[0] = vc[0].real();
        epipole[1] = vc[1].real();
        epipole /= double(vc[2].real());
      }

    return epipole.cast<float>();
  }

  // http://www.cse.psu.edu/~rcollins/CSE486/lecture19_6pp.pdf
  // Page 5.
  inline vfloat2 epipole_right(const Eigen::Matrix3f& F)
  {
    // e_r F  = 0.
    Eigen::EigenSolver<Eigen::Matrix3d> es(F.cast<double>() * F.cast<double>().transpose());


    auto values = es.eigenvalues();
    auto vectors = es.eigenvectors();
    vdouble2 epipole;
    double min_ev = FLT_MAX;
    for (int i = 0; i < values.size(); i++)
      if (values[i].real() < min_ev)
      {
        min_ev = values[i].real();
        auto vc = vectors.col(i);
        epipole[0] = vc[0].real();
        epipole[1] = vc[1].real();
        epipole /= double(vc[2].real());
      }

    return epipole.cast<float>();
  }
}
