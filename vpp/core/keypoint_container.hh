#ifndef VPP_CORE_KEYPOINT_CONTAINER_HH_
# define VPP_CORE_KEYPOINT_CONTAINER_HH_

namespace vpp
{

  template <typename P, typename F>
  struct keypoint_container
  {
    typedef P keypoint_type;
    typedef F feature_type;

    typedef std::vector<P> point_vector_type;
    typedef std::vector<F> feature_vector_type;

    keypoint_container(const box2d& d);

    void compact();
    void swap_buffers();

    template <typename T, typename D>
    void sync_attributes(T& container, typename T::value_type new_value = typename T::value_type(),
			 D die_fun = default_die_fun<typename T::value_type>()) const;

    void add(const keypoint_type& p, const feature_type& f);
    void remove(int i);
    void remove(vint2 pos);

    void update(unsigned i, const keypoint_type& p, const feature_type& f);

    point_vector_type&  keypoints() { return point_vector_; }
    const point_vector& keypoints() const  { return point_vector_; }
    image2d<int>&       keypoint_index2d()  { return index2d_; }
    const image2d<int>& keypoint_index2d() const  { return index2d_; }

    int size() const { return particles_vec_.size(); }

  private:
    image2d<int> index2d_;
    point_vector_type keypoint_vector_;
    feature_vector_type feature_vector_;
  };

}

# include <vpp/core/keypoint_container.hpp>

#endif
