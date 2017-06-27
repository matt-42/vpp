#pragma once

#include <Eigen/Dense>
#include <vpp/algorithms/symbols.hh>

namespace vpp
{

  namespace lk_internals
  {
    template <typename F, typename GD>
    auto match(vfloat2 p, vfloat2 tr_prediction,
	       F A, F B, GD Ag,
	       const int winsize,
	       const float min_ev_th,
	       const int max_interations,
	       const float convergence_delta)
    {
      typedef typename F::value_type V;
      int WS = winsize;
      int ws = winsize;
      int hws = ws/2;

      // Gradient matrix
      Eigen::Matrix2f G = Eigen::Matrix2f::Zero();
      int cpt = 0;
      for(int r = -hws; r <= hws; r++)
	for(int c = -hws; c <= hws; c++)
	  {
	    vfloat2 n = p + vfloat2(r, c);
	    if (A.has(n.cast<int>()))
	      {
		Eigen::Matrix2f m;
		auto g = Ag.linear_interpolate(n);
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
      auto ev = (G / cpt).eigenvalues();
      for (int i = 0; i < ev.size(); i++)
	if (fabs(ev[i].real()) < min_ev) min_ev = fabs(ev[i].real());

      if (min_ev < min_ev_th)
	return std::pair<vfloat2, float>(vfloat2(-1,-1), FLT_MAX);

      Eigen::Matrix2f G1 = G.inverse();

      // Precompute gs and as.
      vfloat2 prediction_ = p + tr_prediction;
      vfloat2 v = prediction_;
      Eigen::Vector2f nk = Eigen::Vector2f::Ones();

      char gs_buffer[WS * WS * sizeof(vfloat2)];
      vfloat2* gs = (vfloat2*) gs_buffer;
      // was: vfloat2 gs[WS * WS];

      typedef plus_promotion<V> S;
      char as_buffer[WS * WS * sizeof(S)];
      S* as = (S*) as_buffer;
      // was: S as[WS * WS];
      {
	for(int i = 0, r = -hws; r <= hws; r++)
	  {
	    for(int c = -hws; c <= hws; c++)
	      {
		vfloat2 n = p + vfloat2(r, c);
		if (Ag.has(n.cast<int>()))
		  {
		    gs[i] = Ag.linear_interpolate(n).template cast<float>();
		    as[i] = cast<S>(A.linear_interpolate(n));
		  }
		i++;
	      }
	  }
      }
      auto domain = B.domain();// - border(hws + 1);

      // Gradient descent
      for (int k = 0; k <= max_interations && nk.norm() >= convergence_delta; k++)
	{
	  Eigen::Vector2f bk = Eigen::Vector2f::Zero();
	  // Temporal difference.
	  int i = 0;
	  for(int r = -hws; r <= hws; r++)
	    {
	      for(int c = -hws; c <= hws; c++)          
		{
		  vfloat2 n = p + vfloat2(r, c);
		  if (Ag.has(n.cast<int>()))
		    {
		      vfloat2 n2 = v + vfloat2(r, c);
		      auto g = gs[i];
		      float dt = (cast<float>(as[i]) - cast<float>(B.linear_interpolate(n2)));
		      bk += Eigen::Vector2f{g[0] * dt, g[1] * dt};
		    }
		  i++;
		}
	    }

	  nk = G1 * bk;
	  v += vfloat2{nk[0], nk[1]};

	  if (!domain.has(v.cast<int>()))
	    return std::pair<vfloat2, float>(vfloat2(0, 0), FLT_MAX);
	}

      // Compute the SSD.
      float err = 0;
      for(int r = -hws; r <= hws; r++)
	for(int c = -hws; c <= hws; c++)
	  {
	    vfloat2 n2 = v + vfloat2(r, c);
	    int i = (r+hws) * ws + (c+hws);
	    {
	      err += fabs(cast<float>(as[i] - cast<S>(B.linear_interpolate(n2))));
	      cpt++;
	    }
	  }

      return std::pair<vfloat2, float>(v - p, err / (cpt));


    }
  }
  
  template <typename V, typename... OPTS>
  void lucas_kanade(const image2d<V>& i1,
		    const image2d<V>& i2,
		    OPTS... opts)
  {
    auto options = iod::D(opts...);
    int niterations = options.get(_niterations, 21);
    int winsize = options.get(_winsize, 11);
    int nscales = options.get(_nscales, 3);
    int min_ev = options.get(_min_ev, 0.0001);
    int delta = options.get(_delta, 0.1);
    auto prediction = options.get(_prediction,
				  [] (auto p) { return vfloat2(0.f, 0.f); });
    auto flow = options.flow;

    auto keypoints = options.keypoints;

    typedef std::decay_t<decltype(V() - V())> Gr;
    pyramid2d<V> pyramid_prev(i1, nscales, 2, _border = winsize / 2);
    pyramid2d<vector<Gr, 2>> pyramid_prev_grad(i1.domain(), nscales, 2, _border = winsize / 2);
    pyramid2d<V> pyramid_next(i2, nscales, 2, _border = winsize / 2);

    scharr(pyramid_prev[0], pyramid_prev_grad[0]);
    pyramid_prev_grad.propagate_level0();

    for (int i = 0; i < keypoints.size(); i++)
    {
      auto kp = keypoints[i];

      vfloat2 tr = (prediction(kp)).template cast<float>() / float(std::pow(2, nscales));
      float dist = 0.f;
      for(int S = pyramid_prev.size() - 1; S >= 0; S--)
	{
	  tr *= pyramid_prev.factor();
	  auto match = lk_internals::match(kp.template cast<float>() / int(std::pow(2, S)),
					   tr,
					   pyramid_prev[S],
					   pyramid_next[S],
					   pyramid_prev_grad[S],
					   winsize,
					   min_ev,
					   niterations, delta);

	  tr = match.first;
	  dist = match.second;
	}

      flow(kp, tr, dist);

    }
  }

}
