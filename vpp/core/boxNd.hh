#ifndef VPP_Boxnd_HH__
# define VPP_Boxnd_HH__

# include <vpp/core/vector.hh>
# include <vpp/core/boxNd_iterator.hh>

namespace vpp
{

  template <unsigned N, typename C = int>
  class boxNd
  {
  public:
    typedef vector<C, N> coord_type;
    typedef boxNd_iterator<N, C> iterator;
    typedef boxNd_row_iterator<N, C> row_iterator;

    inline boxNd() : p1_(), p2_() {}

    inline boxNd(coord_type p1, coord_type p2)
      : p1_(p1),
        p2_(p2)
    {
    }

    inline bool has(const coord_type& p) const
    {
      for (unsigned i = 0; i < N; i++)
        if (p[i] < p1_[i] || p[i] > p2_[i])
          return false;
      return true;
    }

    inline boxNd_iterator<N, C> begin() const
    {
      return boxNd_iterator<N, C>(p1_, *this);
    }

    inline boxNd_iterator<N, C> end() const
    {
      coord_type e = p1_;
      e[0] = p2_[0] + 1;
      return boxNd_iterator<N, C>(e, *this);
    }

    inline const coord_type& p1() const { return p1_; }
    inline const coord_type& p2() const { return p2_; }

    inline coord_type& p1() { return p1_; }
    inline coord_type& p2() { return p2_; }

    inline int size(int d) const {
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

  class border
  {
  public:
    inline border(int n) : size_(n) {}
    inline int size() const { return size_; }
  private:
    int size_;
  };

  template <unsigned N, typename C>
  boxNd<N, C> operator-(const boxNd<N, C>& b, const border& border)
  {
    boxNd<N, C> res = b;

    for (int n = 0; n < N; n++)
    {
      res.p1()[n] += border.size();
      res.p2()[n] -= border.size();
    }

    return res;
  }


  template <unsigned N, typename C>
  boxNd<N, C> operator+(const boxNd<N, C>& b, const border& border)
  {
    boxNd<N, C> res = b;

    for (int n = 0; n < N; n++)
    {
      res.p1()[n] -= border.size();
      res.p2()[n] += border.size();
    }

    return res;
  }

};

#endif
