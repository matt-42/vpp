#ifndef VPP_CORE_KEYPOINT_CONTAINER_HPP_
# define VPP_CORE_KEYPOINT_CONTAINER_HPP_

#include <vpp/core/keypoint_container.hh>

namespace vpp
{

  template <typename P, typename F>
  keypoint_container<P, F>::keypoint_container(const box2d& d)
    : index2d_(d, border(10))
  {
    fill_with_border(index2d_, -1);
    keypoint_vector_.reserve((d.nrows() * d.ncols()) / 10);
    feature_vector_.reserve((d.nrows() * d.ncols()) / 10);
    compact_has_run_ = false;
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::compact()
  {
    compact_has_run_ = true;

    matches_.resize(keypoint_vector_.size());
    std::fill(matches_.begin(), matches_.end(), -1);

    auto pts_it   = keypoint_vector_.begin();
    auto feat_it  = feature_vector_.begin();
    auto pts_res  = keypoint_vector_.begin();
    auto feat_res = feature_vector_.begin();

    for (;pts_it != keypoint_vector_.end();)
    {
      if (pts_it->alive())
      {
        *pts_res++ = *pts_it;
        *feat_res++ = *feat_it;
        int prev_index = pts_it - keypoint_vector_.begin();
        int new_index = pts_res - keypoint_vector_.begin() - 1;
        index2d_(cast<vint2>(pts_it->position)) = new_index;
        matches_[prev_index] = new_index;
        assert(keypoint_vector_[index2d_(pts_it->position)].position == pts_it->position);
      }

      pts_it++;
      feat_it++;
    }

    keypoint_vector_.resize(pts_res - keypoint_vector_.begin());
    feature_vector_.resize(feat_res - feature_vector_.begin());
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::prepare_matching()
  {
    fill_with_border(index2d_, -1);
  }

  // template <typename P, typename F>
  // template <typename T, typename D>
  // void
  // keypoint_container<P, F>::sync_attributes(T& container,
  //                                     typename T::value_type new_value,
  //                                     D die_fun) const
  // {
  // }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::add(const keypoint_type& p,
                                const feature_type& f)
  {
    index2d_(cast<vint2>(p.position)) = keypoint_vector_.size();
    keypoint_vector_.push_back(p);
    feature_vector_.push_back(f);
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::remove(int i)
  {
    assert(i < size());
    keypoint_vector_[i].die();
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::remove(vint2 position)
  {
    assert(has(position));
    remove(index2d_(position));
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::update(unsigned i, const keypoint_type& p, const feature_type& f)
  {
    assert(i < size());
    assert(i >= 0);

    keypoint_vector_[i] = p;
    feature_vector_[i] = f;

    index2d_(p.position) = i;
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::update_index(unsigned i, const vint2& p)
  {
    index2d_(p) = i;
  }

  template <typename P, typename F>
  bool
  keypoint_container<P, F>::has(vint2 p) const
  {
    return index2d_(p) >= 0;
  }

  template <typename P, typename F>
  int
  keypoint_container<P, F>::size() const
  {
    return keypoint_vector_.size();
  }

}


#endif
