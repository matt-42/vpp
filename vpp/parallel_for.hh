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

  template <typename D>
  parallel_for_runner<D, openmp> parallel_for_openmp(D domain)
  {
    return parallel_for_runner<D, openmp>(domain);
  }

  template <typename D, typename B>
  parallel_for_runner<D, B> parallel_for(D domain, B backend)
  {
    return parallel_for_runner<D, B>(domain);
  }


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

};

#endif
