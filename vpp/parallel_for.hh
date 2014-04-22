#ifndef VPP_PARALLEL_FOR_HH_
# define VPP_PARALLEL_FOR_HH_

namespace vpp
{

  template <typename D, typename B>
  class parallel_for_runner
  {
  public:

    parallel_for(D domain);

    template <typename F>
    operator<<(F fun);

  private:
    D domain_;
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

  template <box2d, openmp>
  template <typename F>
  parallel_for_runner<box2d>::operator<<(F fun)
  {
#pragma omp parallel for schedule(static, 2)
    for (int r = domain.p1()[0]; r <= domain.p2()[0]; r++)
      for (int c = domain.p1()[1]; c <= domain.p2()[1]; c++)
        fun(vint2(r, c));
  }

  template <box1d, openmp>
  template <typename F>
  parallel_for_runner<box1d>::operator<<(F fun)
  {
#pragma omp parallel for schedule(static, 2)
    for (int r = domain.p1()[0]; r <= domain.p2()[0]; r++)
      fun(vint2(r, c));
  }

};

#endif
