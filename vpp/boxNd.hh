#ifndef VPP_Boxnd_HH__
# define VPP_Boxnd_HH__

# include <vpp/vector.hh>
# include <vpp/boxNd_iterator.hh>

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

    boxNd_iterator<N, C> begin() const
    {
      return boxNd_iterator<N, C>(p1_, *this);
    }

    boxNd_iterator<N, C> end() const
    {
      coord_type e = p1_;
      e[0] = p2_[0] + 1;
      return boxNd_iterator<N, C>(e, *this);
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

  template <unsigned N, typename C>
  bool operator==(const boxNd<N, C>& a, const boxNd<N, C>& b)
  {
    return a.p1() == b.p1() && a.p2() == b.p2();
  }

  template <unsigned N, typename C>
  bool operator!=(const boxNd<N, C>& a, const boxNd<N, C>& b)
  {
    return !(a == b);
  }

  typedef boxNd<1> box1d;
  typedef boxNd<2> box2d;
  typedef boxNd<3> box3d;
  typedef boxNd<4> box4d;

  inline box2d make_box2d(int nr, int nc)
  {
    return box2d(vint2(0, 0), vint2(nr - 1, nc - 1));
  }

  inline box3d make_box3d(int ns, int nr, int nc)
  {
    return box3d(vint3(0, 0, 0), vint3(ns - 1, nr - 1, nc - 1));
  }


};

#endif
