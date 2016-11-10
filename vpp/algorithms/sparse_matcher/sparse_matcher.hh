#pragma once

namespace vpp
{
  // Match two sets of keypoint using the SAD (sum of absolute differences) distance.
  // F::operator[](int i) must return the ith component of the descriptor.
  template <typename F>
  auto sad_local_sparse_matcher(vpp::box2d domain,
                                const std::vector<vpp::vint2>& kps1,
                                const std::vector<vpp::vint2>& kps2,
                                const std::vector<F>& features1,
                                const std::vector<F>& features2,
                                int search_radius);

  template <typename T>
  struct bucket2
  {
    bucket2(int index_size, int max_avg) : max_avg_(max_avg), avg_index_(index_size, -1)
      {}
    void push_back(const T& t, int avg)
    {
      points_.push_back(std::make_pair(t, avg));
    }

    void finalize()
    {
      std::sort(points_.begin(), points_.end(), [] (auto x, auto y) { return x.second < y.second; });
      avg_index_[0] = 0;

      for(int i = 0; i < points_.size(); i++)
      {
        if (index_of_avg(points_[i].second) == -1)
        {
          index_of_avg(points_[i].second) = i;
        }
      }

      for(int i = 1; i < avg_index_.size(); i++)
        if (avg_index_[i] == -1)
          avg_index_[i] = avg_index_[i - 1];
    }

    int& index_of_avg(distance_t a)
    {
      return avg_index_[std::min(int(avg_index_.size() - 1),
                                 std::max(0, int(int(avg_index_.size() - 1) * (a) / max_avg_// (APPROX*FSIZE*255)
                                            )))];
    }
    
    void reset() { points_ = std::vector<T>(); }

    T& back() { return points_.back(); }
    bool full() const { return false; }
    unsigned size() { return points_.size(); }

    T& operator[] (int i) { return points_[i].first; }
    
    template <typename C>
    void apply_near(vpp::vint2 prediction, distance_t avg,
                    int search_radius, distance_t& max_avg_distance,
                    C fun)
    {
      int avg_prediction = index_of_avg(avg);
      fun(points_[avg_prediction].first);
      // int start = index_of_avg(avg - std::min(max_avg_distance, 9999999));

      // V3
      bool done = false;
      for (int i = avg_prediction - 1, j = avg_prediction + 1; !done; i--, j++)
      {

        done = true;
        if (i >= 0 and (avg - points_[i].second) <= max_avg_distance)
        {
          done = false;
          if ((prediction - points_[i].first.pos).norm() < search_radius)
            fun(points_[i].first);
        }

        if (j < points_.size() and (points_[j].second - avg) <= max_avg_distance)
        { 
          done = false;
          if ((prediction - points_[j].first.pos).norm() < search_radius)
            fun(points_[j].first);
        }
      }
    }

    
  private:
    const int max_avg_;
    std::vector<std::pair<T, distance_t>> points_;
    std::vector<int> avg_index_;
  };
  
  struct keypoint_index
  {

    struct PI { vpp::vint2 pos; int idx; // feature_type f;
    };

    typedef bucket2<PI> bt;

    keypoint_index(vpp::box2d d, int grid_size, int max_avg, int bucket_index_size)
      : S(grid_size),
        idx(1 + S,  1 + S * d.ncols() / d.nrows()),
        s(std::ceil(float(d.nrows()) / S))
      
    {
      pixel_wise(idx) | [&] (auto& i) { new (&i) bt(bucket_index_size, max_avg); };
    }

    void finalize()
    {
      pixel_wise(idx) | [] (auto& i) { i.finalize(); };
    }
    
    void add(vpp::vint2 p, int i, distance_t avg, const feature_type& f)
    {
      if (S > 1)
      {
        if (idx.has(p / s))
        {
          auto& b = idx(p / s);
          b.push_back(PI{p, i}, avg);
        }
      }
      else
      {
        idx(0,0).push_back(PI{p, i}, avg);
      }

      // if (idx.has(p / s))
      // {
      //   auto& b = idx(p / s);
      //   if (!b.full()) b.push_back(PI{p, i}, avg);
      // }
    }

    template <typename C>
    void apply_near(vpp::vint2 prediction, distance_t avg, int search_radius,
                    distance_t& best_distance, C fun)
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

  template <typename F>
  distance_t sad_distance(const F& i1, const F& i2, distance_t th)
  {
    const int RS = 128;
    distance_t err = 0.f;
    for (int r = 0; r < (i1.size())/RS and err <= th; r++)
    {
      for (int i = 0; i < RS; i++)
        err += std::abs((i1.data[r * RS + i]) - (i2.data[r * RS + i]));
    }

    if (err <= th)
    {
      for (int i = (((i1.size())/RS)*RS); i < i1.size(); i++)
        err += std::abs((i1.data[i]) - (i2.data[i]));
    }

    return err;
  }


  template <typename Ki, typename F, typename M, typename G>
  void sparse_matcher(vpp::box2d domain,
                      P1 kps1,
                      P2 kps2,
                      int search_radius,
                      M match_callback,
                      G distance)
  {
    Ki k_index(domain, grid_size, max_avg, bucket_index_size);
    for(int i = 0; i < next_kps.size(); i++)
      k_index.add(next_kps[i], i, features2[i].avg, features2[i]);

    k_index.finalize();
    int sum_ssd_calls = 0;
    int sum_n_tests_before_min = 0;
    int max_ssd_calls = 0;

#pragma omp parallel for
    for(int i = 0; i < prev_kps.size(); i++)
    {
      vpp::vint2 p = prev_kps[i];
    
      typedef keypoint_index::PI PI;
      PI best_n;
      distance_t best_distance = DMAX;

      feature_type f1 = features1[i];
      int ssd_calls = 0;
      int n_tests_before_min = 0;
      auto test_keypoint = [&] (const PI& n)
        {
          distance_t d = ssd_distance(f1, features2[n.idx], best_distance);
          ssd_calls++;
          // std::cout << "     d : "<< d << std::endl;
          if (d < best_distance)
          {
            n_tests_before_min = ssd_calls;
            best_n = n;
            best_distance = d;
          }
        };

      k_index.apply_near(p, features1[i].avg, search_radius, best_distance, test_keypoint);

      if (best_distance != DMAX)
        match_callback(i, best_n.idx, best_distance);
    }
  }

  struct view_as_vector_of_struct
  {
    auto operator[](int i) { return std::forward_as_reference(); }
  };
              
  
  template <typename F>
  auto sparse_matcher(vpp::box2d domain,
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
           return ssd_distance(features1[i], features2[j], th); });
    
    return M;
  }
  
}
