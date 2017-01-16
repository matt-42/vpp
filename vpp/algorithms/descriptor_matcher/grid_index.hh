#pragma once

#include <vpp/core/image2d.hh>
#include <vpp/core/pixel_wise.hh>
#include <vpp/algorithms/symbols.hh>

namespace vpp
{

  template <typename SI>
  struct grid_index
  {

    template <typename O>
    grid_index(O&& o) :
      s(o.get(s::_cell_width, 300)),
      search_radius(o.get(s::_search_radius, 300))
    {
      vint2 pmax(0,0);
      for (vint2 p : o.train_positions)
	{
	  pmax[0] = std::max(pmax[0], p[0]);
	  pmax[1] = std::max(pmax[1], p[1]);
	}

      idx = vpp::image2d<SI>(int(std::ceil((1 + pmax[0]) / float(s))),
			     int(std::ceil((1 + pmax[1]) / float(s))));

      for (vint2 p : o.train_positions)
	assert(idx.has(p / s));

      pixel_wise(idx) | [&] (SI& si)
	{
	  new (&si) SI(o);
	};
    }
    
    template <typename O>
    void index(O&& o)
    {
      assert(idx.has(o.position / s));
      idx(o.position / s).index(o);
    }

    void finalize()
    {

      pixel_wise(idx) | [&] (SI& si)
      {
        si.finalize();
      };

    }
    
    template <typename O, typename F>
    auto search(O&& o, F distance)
    {
      vint2 prediction = o.position;

      vint2 begin = (prediction - vint2(search_radius, search_radius)) / s;
      vint2 end = (prediction + vint2(search_radius, search_radius)) / s;

      typedef decltype(distance(0, 0, 0)) distance_t;

      int best_idx = -1;
      distance_t best_distance = std::numeric_limits<distance_t>::max();

      for (vint2 n : box2d(begin, end))
      {
        if (idx.has(n) and idx(n).size() > 0)
        {
          auto& b = idx(n);
          auto m = b.search(o, distance);
	  if (m.distance < best_distance)
	    {
	      best_distance = m.distance;
	      best_idx = m.idx;
	    }
        }
      }

      return iod::D(_distance = best_distance, _idx = best_idx);
    }

    vpp::image2d<SI> idx;
    int s; // scale
    int search_radius;
  };

}
