#pragma once

#include <vpp/vpp.hh>

namespace vpp
{

  template <typename D>
  inline
  auto gradient_descent_match(const vint2 p,
                              vint2 prediction,
                              D distance,
                              int max_iteration = 10)
  {
    typedef std::pair<vint2, float> ret;
    vint2 match = prediction;
    int match_distance = distance(p, prediction, INT_MAX);
    unsigned match_i = 8;

    const int c8_it[9][2] =
      {
        /*
          0  1  2
          7     3
          6  5  4
        */

        {6, 3}, // 0
        {0, 3}, // 1
        {0, 5}, // 2
        {2, 5}, // 3
        {2, 7}, // 4
        {4, 7}, // 5
        {4, 1}, // 6
        {6, 1}, // 7
        {0, 0} // 8
      };

    const int c8[8][2] =
      {
        {-1, 1}, {0, 1}, {1, 1},
        {-1, 0},         {1, 0},
        {-1, -1}, {0, -1}, {1, -1}
      };
    
   for (int search = 0; search < max_iteration; search++)
    {
      int i = c8_it[match_i][0];
      int end = c8_it[match_i][1];
      
      // First lookup
      {
        vint2 n(prediction + vint2(c8[i]));

        int d = distance(p, n, match_distance);
        if (d < match_distance)
        {
          match = n;
          match_i = i;
          match_distance = d;
        }
        i = (i + 1) & 7;
      }


      // Loop over the non visited neighbors
#pragma unroll 4
      for(; i != end; i = (i + 1) & 7)
      {
        vint2 n(prediction + vint2(c8[i]));
        int d = distance(p, n, match_distance);
        if (d < match_distance)
        {
          match = n;
          match_i = i;
          match_distance = d;
          //break;
        }
      }

      if (vint2(prediction) == vint2(match))
        break; // Local minimum found.
      else
        prediction = match;

    }

   return iod::D(_flow = vint2(match - p), _distance = match_distance);
  }

  
}
