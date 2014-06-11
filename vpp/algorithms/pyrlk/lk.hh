#ifndef VPP_ALGORITHMS_PYRLK_LK_HH_
# define VPP_ALGORITHMS_PYRLK_LK_HH_

# include <Eigen/Dense>
# include <cfloat>

namespace vpp
{
  template <unsigned WS>
  struct lk_match_point_square_win
  {
    template <typename F, typename GD>
    std::pair<vfloat2, float> operator()(vfloat2 p, vfloat2 tr_prediction,
                                         F A, F B, GD Ag,
                                         float min_ev_th)
    {
      typedef typename F::value_type V;
      int ws = WS;
      int hws = ws/2;

      int factor = 1;

      // Gradient matrix
      Eigen::Matrix2f G = Eigen::Matrix2f::Zero();
      int cpt = 0;
      for(int r = -hws; r <= hws; r++)
        for(int c = -hws; c <= hws; c++)
        {
          vfloat2 n = p + vfloat2(r, c) * factor;
          if (A.has(n.cast<int>()))
          {
            Eigen::Matrix2f m;
            vfloat2 g = Ag.linear_interpolate(n);
            float gx = g[0];
            float gy = g[1];
            m <<
              gx * gx, gx * gy,
              gx * gy, gy * gy;
            G += m;
            cpt++;
          }
        }

      // Check minimum eigenvalue.
      float min_ev = 99999.f;
      auto ev = G.eigenvalues();
      for (int i = 0; i < ev.size(); i++)
        if (fabs(ev[i].real()) < min_ev) min_ev = fabs(ev[i].real());

      std::cout << min_ev << std::endl;
      if (min_ev < min_ev_th)
        return std::pair<vfloat2, float>(vfloat2(-1,-1), FLT_MAX);

      Eigen::Matrix2f G1 = G.inverse();

      // Precompute gs and as.
      vfloat2 prediction_ = p + tr_prediction;
      vfloat2 v = prediction_;
      Eigen::Vector2f nk = Eigen::Vector2f::Ones();

      vfloat2 gs[ws * ws];
      V as[ws * ws];
      {
        for(int i = 0, r = -hws; r <= hws; r++)
        {
          for(int c = -hws; c <= hws; c++)
          {
            vfloat2 n = p + vfloat2(r, c) * factor;
            if (Ag.has(n.cast<int>()))
            {
              gs[i] = Ag.linear_interpolate(n);
              as[i] = A.linear_interpolate(n);
            }
            i++;
          }
        }
      }
      auto domain = B.domain() - border(3);

      // Gradient descent
      for (int k = 0; k < 30 && nk.norm() >= 0.1; k++)
      {
        Eigen::Vector2f bk = Eigen::Vector2f::Zero();
        // Temporal difference.
        cpt = 0;
        int i = 0;
        for(int r = -hws; r <= hws; r++)
        {
          for(int c = -hws; c <= hws; c++)
          {
            vfloat2 n2 = v + vfloat2(r, c) * factor;
            {
              auto g = gs[i];
              //std::cout << "as:" << as[i] << std::endl;
              //std::cout << "B:" << B.linear_interpolate(n2)[0] << std::endl;

              float dt = (as[i] - B.linear_interpolate(n2))[0];
              //std::cout << "dt:" << dt << std::endl;
              bk += Eigen::Vector2f(g[0] * dt, g[1] * dt);
              cpt++;
            }
            i++;
          }
        }

        nk = G1 * bk;
        if (nk.norm() > ws) return std::pair<vfloat2, float>(vfloat2(-1,-1), FLT_MAX);
        v += vfloat2(nk[0], nk[1]);

        std::cout << "v: " << v.transpose() << std::endl;
        if (!domain.has(v.cast<int>()))
          return std::pair<vfloat2, float>(vfloat2(-1,-1), FLT_MAX);
      }

      // Compute matching error as the ssd.
      float err = 0;
      for(int r = -hws; r <= hws; r++)
        for(int c = -hws; c <= hws; c++)
        {
          vfloat2 n2 = v + vfloat2(r, c) * factor;
          int i = (r+hws) * hws + (c+hws);
          {
            std::cout << n2.transpose() << std::endl;
            std::cout << " err: "  << as[i].cast<int>() << " - " << B.linear_interpolate(n2).cast<int>() << std::endl;
            std::cout << " err: "  << B(n2.cast<int>()).cast<int>() << std::endl;
            err += fabs(cast<float>(as[i] - B.linear_interpolate(n2)));
            cpt++;
          }
        }

      //std::cout << "v: " << (v - p).transpose() << " " << err / cpt << std::endl;

      return std::pair<vfloat2, float>(v - p, err / cpt);

    }
  };
}

#endif
