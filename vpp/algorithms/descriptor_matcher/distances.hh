#pragma once

#include <cstdlib>
#include <cmath>
#include <iod/array_view.hh>

namespace vpp
{

  template <typename D, typename F>
  D sad_distance(F&& i1, F&& i2, D th)
  {
    const int RS = 128;
    D err = 0;
    for (int r = 0; r < (i1.size())/RS and err <= th; r++)
    {
      for (int i = 0; i < RS; i++)
        err += std::abs((i1[r * RS + i]) - (i2[r * RS + i]));
    }

    
    if (err <= th)
    {
      for (int i = (((i1.size())/RS)*RS); i < i1.size(); i++)
        err += std::abs((i1[i]) - (i2[i]));
    }

    // // for (int i = 0; i < i1.size(); i++)
    // //   err += std::abs((i1[i]) - (i2[i]));
    
    return err;
  }

  template <typename D, typename F>
  D sad_distance(int size, F&& i1, F&& i2, D th)
  {
    return sad_distance(iod::array_view(size, i1), iod::array_view(size, i2), th);
  }

  template <typename D, typename F>
  D sad_distance(int size, F&& i1, F&& i2)
  {
    return sad_distance(iod::array_view(size, i1), iod::array_view(size, i2),
                        std::numeric_limits<D>::max());
  }

  template <typename D, typename F>
  D sad_distance(F&& i1, F&& i2)
  {
    return sad_distance(i1, i2, std::numeric_limits<D>::max());
  }
  
}
