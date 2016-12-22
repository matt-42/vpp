#pragma once

#include <iod/sio_utils.hh>

namespace vpp
{

  inline vfloat2 line2d_to_direction_vector(const vfloat3& line)
  {
    vfloat2 dv;
    if (line[1] == 0)
      dv = vfloat2(0, 1);
    else
    {
      dv = vfloat2(1, -line[0]/line[1]);
      dv.normalize();
    }

    return dv;
  }

  template <typename C, typename D>
  auto epipolar_match(vector<C, 2> p1, vector<C, 2> prediction, vfloat2 epipole,
                      Eigen::Matrix3f F, D distance)
  {
    vfloat2 v = line2d_to_direction_vector(F * p1.template cast<float>().homogeneous());
    float d = (prediction.template cast<float>() - epipole).dot(v);

    float cost = distance(p1, (epipole + d * v).template cast<C>(), INT_MAX);
    float costA = distance(p1, (epipole + (d + 1.5) * v).template cast<C>(), INT_MAX);
    float costB = distance(p1, (epipole + (d - 1.5) * v).template cast<C>(), INT_MAX);

    if (cost >= costA or cost >= costB)
    {
      float dir = costA < costB ? +1.5 : -1.5;
      float new_cost = std::min(costA, costB);

      while (new_cost <= cost and cost != FLT_MAX)
      {
        d += dir;
        cost = new_cost;
        new_cost = distance(p1, (epipole + (d + dir) * v).template cast<C>(), cost);
      }
    }

    return iod::D(_flow = (epipole + d * v).template cast<C>(),
                  _distance = cost);

  }

}
