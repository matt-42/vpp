#ifndef VPP_Boxnd_HH__
# define VPP_Boxnd_HH__

# include <vpp/vector.hh>

namespace vpp
{

  template <unsigned N, typename C = int>
  class boxNd
  {
  public:
    typedef vector<C, N> coord_type;

    boxNd() : p1_(), p2_() {}

    boxNd(coord_type p1, coord_type p2)
      : p1_(p1),
        p2_(p2)
    {
    }

    bool has(const coord_type& p) const
    {
      for (unsigned i = 0; i < N; i++)
        if (p[i] < p1_[i] || p[i] > p2_[i])
          return false;
      return true;
    }

    const coord_type& p1() const { return p1_; }
    const coord_type& p2() const { return p2_; }

    coord_type& p1() { return p1_; }
    coord_type& p2() { return p2_; }

    int size(int d) const {
      assert(d >= 0);
      assert(d < N);
      return p2_[d] - p1_[d] + 1;
    }

  private:
    coord_type p1_;
    coord_type p2_;
  };

  typedef boxNd<1> box1d;
  typedef boxNd<2> box2d;
  typedef boxNd<3> box3d;
  typedef boxNd<4> box4d;
};

#endif
