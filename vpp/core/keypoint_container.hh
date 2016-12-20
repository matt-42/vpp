#ifndef VPP_CORE_KEYPOINT_CONTAINER_HH_
# define VPP_CORE_KEYPOINT_CONTAINER_HH_

# include <vpp/core/vector.hh>
# include <vpp/core/image2d.hh>
# include <vpp/core/zero.hh>


namespace vpp
{

  template <typename C>
  struct keypoint
  {
    keypoint() : age(0) {}
    keypoint(vector<C, 2> pos) : position(pos), velocity(0,0),
                                 age(1) {}

    vector<C, 2> position;
    vector<C, 2> velocity;
    int age;

    void die() { age = 0; }
    bool alive() { return age > 0; }
  };

  template <typename P, typename F>
  struct keypoint_container
  {
    typedef P keypoint_type;
    typedef F feature_type;

    typedef std::vector<P> keypoint_vector_type;
    typedef std::vector<F> feature_vector_type;

    keypoint_container(const box2d& d);

    void compact();
    void prepare_matching();

    struct no_op
    {
      template <typename T>
      void operator()(T& t) {}
    };

    template <typename T, typename D = no_op>
    void sync_attributes(T& container, typename T::value_type new_value = typename T::value_type(),
        		 D die_fun = D()) const;

    template <typename T, typename U>
    void sync_attributes(T& container,
                         typename T::value_type new_value,
                         std::vector<U>& dead_vector) const;

    void add(const vfloat2& p);
    void add(const keypoint_type& p, const feature_type& f = feature_type());
    void remove(int i);
    void remove(vint2 pos);

    template <typename T>
    void move(int i, T pos);
    void update(unsigned i, const keypoint_type& p, const feature_type& f);

    keypoint_vector_type&  keypoints()                 { return keypoint_vector_; }
    const keypoint_vector_type& keypoints() const      { return keypoint_vector_; }
    image2d<int>&       index2d()             { return index2d_; }
    const image2d<int>& index2d() const       { return index2d_; }

    int index_of(vint2& p) const             { return index2d_(p); }

    keypoint_type& operator[] (unsigned i)             { return keypoint_vector_[i]; }
    const keypoint_type& operator[] (unsigned i) const { return keypoint_vector_[i]; }

    keypoint_type& operator() (vint2 p)             { return keypoint_vector_[index2d_(p)]; }
    const keypoint_type& operator() (vint2 p) const { return keypoint_vector_[index2d_(p)]; }

    int size() const;

    void update_index(unsigned i, const vint2& p);

    bool has(vint2 p) const;

  private:
    std::vector<int> matches_;
    image2d<int> index2d_;
    keypoint_vector_type keypoint_vector_;
    feature_vector_type feature_vector_;
    bool compact_has_run_;
  };

}

# include <vpp/core/keypoint_container.hpp>

#endif
