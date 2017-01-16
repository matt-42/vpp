#pragma once

#include <limits>
#include <iod/sio_utils.hh>
#include <vpp/algorithms/symbols.hh>

namespace vpp
{

  template <typename... OPTS>
  void bruteforce_matcher(int query_size, int train_size,
			  OPTS... opts)
  {
    auto options = D(opts...);
    typedef decltype(options) O;

    static_assert(iod::has_symbol<O, s::_distance_t>::value, "bruteforce_matcher: _distance options missing.");
    static_assert(iod::has_symbol<O, s::_match_t>::value, "bruteforce_matcher: _match callback is missing.");
    
    int set1_size = query_size;
    int set2_size = train_size;

    auto distance = options.distance;
    auto match = options.match;

    typedef decltype(distance(0, 0, 0)) distance_t;

    #pragma omp parallel for
    for (int i = 0; i < set1_size; i++)
    {   
      int best_j = 0;
      distance_t best_distance = distance(i, 0, std::numeric_limits<distance_t>::max());

      for (int j = 1; j < set2_size; j++)
      {
        distance_t d = distance(i, j, best_distance);
        if (d < best_distance)
        {
          best_j = j;
          best_distance = d;
        }
      }

#pragma omp critical
      match(i, best_j, best_distance);
    }
  }
}
