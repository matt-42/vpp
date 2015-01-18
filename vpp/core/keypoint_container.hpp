#ifndef VPP_CORE_KEYPOINT_CONTAINER_HPP_
# define VPP_CORE_KEYPOINT_CONTAINER_HPP_

#include <vpp/core/keypoint_container.hh>
#include <vpp/core/fill.hh>

namespace vpp
{

  template <typename P, typename F>
  keypoint_container<P, F>::keypoint_container(const box2d& d)
    : index2d_(d, _border = 10)
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
        assert(keypoint_vector_[index2d_(pts_it->position.template cast<int>())].position == pts_it->position);
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
    compact_has_run_ = false;
    fill_with_border(index2d_, -1);
    std::fill(matches_.begin(), matches_.end(), -1);
  }

  template <typename P, typename F>
  template <typename T, typename D>
  void
  keypoint_container<P, F>::sync_attributes(T& v,
                                            typename T::value_type new_value,
                                            D die_fun) const
  {
    unsigned nparts = keypoint_vector_.size();
    if (compact_has_run_)
    {
      unsigned nmatches = matches_.size();
      T tmp(nparts, new_value);
      for(unsigned i = 0; i < nmatches; i++)
      {
	int ni = matches_[i];
	if (ni >= 0)
	{
	  assert(ni < nparts);
	  assert(keypoint_vector_[ni].age != 1 || i >= v.size());
#ifndef NO_CPP0X
	  if (i < v.size())
	    tmp[ni] = std::move(v[i]);
#else
	  if (i < v.size())
	    tmp[ni] = v[i];
#endif

	}
	else if (ni < 0)
	  die_fun(v[i]);
      }
      v.swap(tmp);
      assert(v.size() == keypoint_vector_.size());
    }
    else
      v.resize(nparts, new_value);
  }

  template <typename P, typename F>
  template <typename T, typename U>
  void
  keypoint_container<P, F>::sync_attributes(T& container,
                                            typename T::value_type new_value,
                                            std::vector<U>& dead_vector) const
  {
    sync_attributes(container, new_value,
                    [&dead_vector] (T& x) { dead_vector.push_back(std::move(x)); });
  }

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
  keypoint_container<P, F>::add(const vfloat2& p)
  {
    keypoint_type kp(p);
    index2d_(cast<vint2>(p)) = keypoint_vector_.size();
    keypoint_vector_.push_back(kp);
    feature_vector_.push_back(feature_type());
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::remove(int i)
  {
    assert(i < size());
    keypoint_vector_[i].die();
    auto& index = index2d_(keypoint_vector_[i].position.template cast<int>());
    if (index == i) index = -1;
  }

  template <typename P, typename F>
  void
  keypoint_container<P, F>::remove(vint2 position)
  {
    assert(has(position));
    remove(index2d_(position));
  }


  template <typename P, typename F>
  template <typename T>
  void
  keypoint_container<P, F>::move(int i, T position)
  {
    assert(i < size());
    assert(i >= 0);

    auto& kp = keypoint_vector_[i];
    kp.velocity = position - kp.position;
    kp.position = position;
    kp.age++;

    index2d_(kp.position.template cast<int>()) = i;
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
