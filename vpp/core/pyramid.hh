#ifndef VPP_PYRAMID_HH__
# define VPP_PYRAMID_HH__


namespace vpp
{

  template <typename V>
  void subsample2(const image2d<V>& in, image2d<V>& out)
  {
    typedef decltype(V() + V()) sum_type;
    auto nbh = box_nbh<V, 2, 2>(in);
    pixel_wise(in, out) << [&] (V& a, V& b)
    {
      sum_type sum = zero<sum_type>();
      nbh(a) << [&] (V& n) { sum += n; };
      b = sum / 4;
    }
  }

  template <typename V, unsigned N>
  struct pyramid
  {
  public:

    typedef imageNd<V, N> image_type;

    pyramid(boxNd<N> d, int nlevels, int factor)
      : levels_(nlevels),
        factor_(factor)
    {
      for (int i = 0; i < nlevels; i++)
      {
        levels[i] = imageNd<V, N>(d);
        d = make_box2d(d.nrows() / factor, d.ncols() / factor);
      }
    }

    void update(const image<V, N>& in)
    {
      copy(in, levels_[0]);
      for (int i = 1; i < levels_.size(); i++)
      {
        if (factor_ == 2)
          subsample2(levels_[i - 1], levels_[i]);
        else
          assert(0); // Not implemented.
      }
    }

    int factor() const
    {
      return factor_;
    }

  private:
    std::vector<image_type> levels_;
    int factor_;
  };

  template <typename V>
  using pyramid2d = pyramid<2, V>;

  template <typename V>
  using pyramid3d = pyramid<3, V>;

};

#endif
