#pragma once

#include <vpp/vpp.hh>

namespace vpp
{

  struct index1d
  {
    struct kp_info
    {
      int id;
      int projection;
    };
  
    
    template <typename O>
    index1d(O&& o)
    {
    }
    
    template <typename O>
    void index(O&& o)
    {
      points_.push_back(kp_info{i, kp, projection});
    }

    void finalize()
    {
      std::sort(points_.begin(), points_.end(),
                [] (auto x, auto y) { return x.projection < y.projection; });
    }

    int position_of_projection(int proj)
    {
      // Dichotomic search.
      int inf = 0;
      int sup = points_.size() - 1;

      while (points_[inf].proj < points_[sup].proj)
      {
        int pivot = (inf + sup) / 2;
        if (points_[pivot].proj < proj)
          inf = pivot;
        else sup = pivot;

        if (points_[inf].proj == proj) sup = inf;
        if (points_[sup].proj == proj) inf = sup;
      }

      return inf;
    }

    template <typename C>
    auto search(O&& o, F distance, int distance_th)
    {
      int projection = project(o.descriptor);
      int projection_idx = index_of_projection(projection);

      int best_idx = projection;
      int best_distance = distance(o.idx, projection, distance_th);
      distance_th = std::min(distance_th, best_distance);
      
      auto test_ith = [&] (int i)
        {
          int dist = distance(o.idx, i, best_distance);
          if (best_distance > dist)
          {
            best_distance = dist;
            best_idx = o.idx;
            distance_th = std::min(distance_th, best_distance);
          }
        };

      bool done = false;
      for (int i = projection_idx - 1, j = projection_idx + 1; !done; i--, j++)
      {
        done = true;
        if (i >= 0 and (approximation * (projection - points_[i].second)) <= best_distance)
        {
          done = false;
          test_ith(points_[i].idx);
        }

        if (j < points_.size() and (approximation * (points_[j].second - projection)) <= best_distance)
        { 
          done = false;
          test_ith(points_[j].idx);
        }
      }
      return D(_idx = best_idx, _distance = best_distance);
    }
    
  private:
    const int max_projection_;
    int approximation_;
    std::vector<kp_info<int>> points_;
    std::vector<int> projection_index_;
  };

}
