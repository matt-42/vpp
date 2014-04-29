#ifndef VPP_PARALLEL_FOR_HH_
# define VPP_PARALLEL_FOR_HH_

# include <vpp/boxNd.hh>

namespace vpp
{
  // Backends
  struct openmp {};
  struct pthreads {}; // Todo
  struct cpp_amp {}; // Todo

  template <typename D, typename B>
  class parallel_for_runner
  {
  public:

    parallel_for_runner(const D& domain) : domain_(domain) {}

    template <typename F>
    void operator<<(F fun);

  private:
    const D& domain_;
  };


  template <>
  template <typename F>
  void
  parallel_for_runner<box3d, openmp>::operator<<(F fun)
  {
    for (int s = domain_.p1()[0]; s <= domain_.p2()[0]; s++)
#pragma omp parallel for schedule(static, 2)
      for (int r = domain_.p1()[1]; r <= domain_.p2()[1]; r++)
        for (int c = domain_.p1()[2]; c <= domain_.p2()[2]; c++)
          fun(vint2(r, c));
  }

  template <>
  template <typename F>
  void
  parallel_for_runner<box2d, openmp>::operator<<(F fun)
  {
#pragma omp parallel for schedule(static, 4)
    for (int r = domain_.p1()[0]; r <= domain_.p2()[0]; r++)
      for (int c = domain_.p1()[1]; c <= domain_.p2()[1]; c++)
        fun(vint2(r, c));
  }

  template <>
  template <typename F>
  void
  parallel_for_runner<box1d, openmp>::operator<<(F fun)
  {
#pragma omp parallel for schedule(static, 50)
    for (int r = domain_.p1()[0]; r <= domain_.p2()[0]; r++)
      fun(vint1(r));
  }


  template <typename I, typename J, typename B>
  class test {
    template <typename F>
    void operator<<(F fun);

  };

  // template <typename I, typename J>
  // class test {

  template <typename I, typename J>
  class test<I, J, openmp>
  {
    template <typename F>
    void operator<<(F fun) {}
  };

  template <typename I, typename J, typename B>
  class parallel_for_pixel_runner
  {
  public:

    //parallel_for_pixel_runner(const I& i1, const J& i2) : i1_(i1), i2_(i2) {}

    template <typename F>
    void operator<<(F fun);

  private:
    const I& i1_;
    const J& i2_;
  };


  template <typename I, typename J>
  class parallel_for_pixel_runner<I, J, openmp>
  {
  public:

    parallel_for_pixel_runner(I& i1, J& i2) : i1_(i1), i2_(i2) {}

    template <typename F>
    void operator<<(F fun)
    {

      //typedef imageNd_iterator<
#pragma omp parallel for schedule(static, 4)
      for (int r = i1_.domain().p1()[0]; r <= i1_.domain().p2()[0]; r++)
      {
        auto i1_cur = typename I::iterator(vint2(r, i1_.domain().p1()[1]), i1_);
        auto i1_end = typename I::iterator(vint2(r, i1_.domain().p2()[1] + 1), i1_);

        auto i2_cur = typename J::iterator(vint2(r, i2_.domain().p1()[1]), i2_);
        auto i2_end = typename J::iterator(vint2(r, i2_.domain().p2()[1] + 1), i2_);

        while (i1_cur != i1_end)
        {
          fun(*i1_cur, *i2_cur);
          i1_cur.next();
          i2_cur.next();
        }
      }
    }

  private:
    I& i1_;
    J& i2_;


  };

  template <typename D>
  parallel_for_runner<D, openmp> parallel_for_openmp(D domain)
  {
    return parallel_for_runner<D, openmp>(domain);
  }

  template <typename I, typename J>
  parallel_for_pixel_runner<I, J, openmp> parallel_for_pixel_openmp(I& i1, J& i2)
  {
    return parallel_for_pixel_runner<I, J, openmp>(i1, i2);
  }

  template <typename D, typename B>
  parallel_for_runner<D, B> parallel_for(D domain, B backend)
  {
    return parallel_for_runner<D, B>(domain);
  }


};

#endif
