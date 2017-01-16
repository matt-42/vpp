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
      approximation_ = o.get(_approximation, 1);
    }

    int size() const { return points_.size(); }

    template <typename O>
    int project(const O& o)
    {
      int projection = 0;
      for (int i = 0; i < o.size(); i++)
	projection += o[i];
      return projection;
    }

    template <typename O>
    void index(O&& o)
    {
      int projection = project(o.descriptor);
      points_.push_back(kp_info{o.idx, projection});
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

      while (((inf + 1) < sup) and points_[inf].projection < points_[sup].projection)
      {
        int pivot = (inf + sup) / 2;
        if (points_[pivot].projection < proj)
          inf = pivot;
        else sup = pivot;

        if (points_[inf].projection == proj) sup = inf;
        if (points_[sup].projection == proj) inf = sup;
      }

      return inf;
    }

    template <typename O, typename F>
    auto search(O&& o, F distance, int distance_th = std::numeric_limits<int>::max())
    {
      int projection = project(o.descriptor);
      int projection_idx = position_of_projection(projection);
      
      int best_idx = points_[projection_idx].id;
      int best_distance = distance(o.idx, best_idx, distance_th);
      distance_th = std::min(distance_th, best_distance);
      
      auto test_ith = [&] (int i)
        {
          int dist = distance(o.idx, i, best_distance);
          if (best_distance > dist)
          {
            best_distance = dist;
            best_idx = i;
            distance_th = std::min(distance_th, best_distance);
          }
        };

      bool done = false;
      for (int i = projection_idx - 1, j = projection_idx + 1; !done; i--, j++)
      {
        done = true;
        if (i >= 0 and (approximation_ * (projection - points_[i].projection)) <= best_distance)
        {
          done = false;
          test_ith(points_[i].id);
        }

        if (j < points_.size() and (approximation_ * (points_[j].projection - projection)) <= best_distance)
        { 
          done = false;
          test_ith(points_[j].id);
        }
      }
      return D(_idx = best_idx, _distance = best_distance);
    }
    
  private:
    int approximation_;
    std::vector<kp_info> points_;
    std::vector<int> projection_index_;
  };

}
