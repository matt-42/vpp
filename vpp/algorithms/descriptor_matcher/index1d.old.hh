#pragma once

#include <vpp/vpp.hh>

namespace vpp
{
  // Match two sets of keypoint using the SAD (sum of absolute differences) distance.
  // kpsN[i] must return the ith component of the descriptor set N.
  // kpsN[i].position must be the 2d coordinates of the ith keypoint.
  // kpsN[i].descriptor[j] must be the jth coordinate of the ith keypoint descriptor.
  //
  // Hint: Use iod::aos_view to build kps1 and kps2.
  //
  template <typename F>
  auto sad_local_sparse_matcher(vpp::box2d domain,
                                const F& kps1,
                                const F& kps2,
                                int search_radius);
  template <typename F>
  struct kp_info
  {
    int id;
    F kp;
    int avg;
  };
  
  template <typename KP>
  struct avg_index
  {
    avg_index(int index_size, int max_avg) : max_avg_(max_avg), avg_index_(index_size, -1)
      {}

    void add(const KP& kp, int i)
    {
      points_.push_back(kp_info{i, kp, avg});
    }

    void finalize()
    {
      for (int i = 0; i < points_.size(); i++)
        max_avg_ = std::max(max_avg_, points_[i].avg;

      std::sort(points_.begin(), points_.end(),
                [] (auto x, auto y) { return x.avg < y.avg; });
      avg_index_[0] = 0;

      for(int i = 0; i < points_.size(); i++)
        if (index_of_avg(points_[i].avg) == -1)
          index_of_avg(points_[i].second) = i;

      for(int i = 1; i < avg_index_.size(); i++)
        if (avg_index_[i] == -1)
          avg_index_[i] = avg_index_[i - 1];
    }

    int& index_of_avg(distance_t a)
    {
      return avg_index_[std::min(int(avg_index_.size() - 1),
                                 std::max(0, int(int(avg_index_.size() - 1) * (a) / max_avg_
                                            )))];
    }
    
    template <typename C>
    void apply_near(vpp::vint2 prediction, distance_t avg,
                    int search_radius, distance_t& max_distance,
                    C fun)
    {
      int avg_prediction = index_of_avg(avg);
      fun(points_[avg_prediction].first);

      // V3
      bool done = false;
      for (int i = avg_prediction - 1, j = avg_prediction + 1; !done; i--, j++)
      {

        done = true;
        if (i >= 0 and (avg - points_[i].second) <= max_distance)
        {
          done = false;
          if ((prediction - points_[i].first.pos).norm() < search_radius)
            fun(points_[i].first);
        }

        if (j < points_.size() and (points_[j].second - avg) <= max_distance)
        { 
          done = false;
          if ((prediction - points_[j].first.pos).norm() < search_radius)
            fun(points_[j].first);
        }
      }
    }
    
  private:
    const int max_avg_;
    std::vector<kp_info<KP>> points_;
    std::vector<int> avg_index_;
  };

  template<typename F>
  struct grid_avg_index
  {
    typedef avg_index<typename F::value_type> bt;

    template <typename F>
    keypoint_index(vpp::box2d d, const F& kps)
    {
      for (int i = 0; i < kps.size(); i++)
        add(kps, i);
      finalize();
    }

    void finalize()
    {
      pixel_wise(idx) | [] (auto& i) { i.finalize(); };
    }

    template <typename KP>
    void add(KP kp, int i)
    {
      idx(p / s).add(kp, i);
    }

    template <typename C>
    void apply_near(vpp::vint2 prediction, int avg, int search_radius,
                    int& best_distance, C fun)
    {
      vint2 begin = (prediction - vint2(search_radius, search_radius)) / s;
      vint2 end = (prediction + vint2(search_radius, search_radius)) / s;
      for (vint2 n : box2d(begin, end))
      {
        if (idx.has(n) and idx(n).size() > 0)
        {
          auto& b = idx(n);
          b.apply_near(prediction, avg, search_radius, best_distance, fun);
        }
      }
    }

    const int S;
    vpp::image2d<bt> idx;
    int s; // scale
  };

  template <typename Ki, typename F, typename M, typename G>
  void sparse_matcher(vpp::box2d domain,
                      const F& kps1,
                      const F& kps2,
                      int search_radius,
                      M match_callback,
                      G distance)
  {
    Ki k_index(domain, kps2);

#pragma omp parallel for
    for(int i = 0; i < prev_kps.size(); i++)
    {
      vpp::vint2 p = prev_kps[i];
    
      typedef keypoint_index::PI PI;
      PI best_n;
      int best_distance = INT_MAX;

      feature_type f1 = features1[i];
      auto test_keypoint = [&] (const PI& n)
        {
          distance_t d = sad_distance(f1, features2[n.idx], best_distance);
          if (d < best_distance)
          {
            best_n = n;
            best_distance = d;
          }
        };

      k_index.apply_near(p, features1[i].avg, search_radius, best_distance, test_keypoint);

      if (best_distance != DMAX)
        match_callback(i, best_n.idx, best_distance);
    }
  }
  
  template <typename F>
  auto sad_local_sparse_matcher(vpp::box2d domain,
                                const std::vector<vpp::vint2>& kps1,
                                const std::vector<vpp::vint2>& kps2,
                                const std::vector<F>& features1,
                                const std::vector<F>& features2,
                                int search_radius)
  {
    const int grid_size = std::max(domain.nrows() / search_radius, domain.cols() / search_radius);
    const int bucket_index_size = 100;

    std::vector<std::pair<int, distance_t>> M(kps1.size(), std::make_pair(-1, DMAX));
    
    fast_sparse_tracker<keypoint_index>
      (domain,
       iod::aos_view(_keypoint = kps1, _descriptor = features1),
       iod::aos_view(_keypoint = kps2, _descriptor = features2),
       search_radius,
       [&] (int i, int j, distance_t d) { M[i] = std::make_pair(j, d); },
       [&] (int i, int j, distance_t th) {
           return sad_distance(features1[i], features2[j], th); });
    
    return M;
  }
  
}
