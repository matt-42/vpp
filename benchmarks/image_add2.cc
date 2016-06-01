
#include <benchmark/benchmark.h>
#include <iostream>
#include <fstream>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <opencv2/opencv.hpp>
#include <mln/core/image/image2d.hh>

#include "get_time.hh"

using namespace vpp;


void raw_naive(vpp::image2d<int> A, image2d<int> B, image2d<int> C)
{
  int* pa = &A(0,0);
  int* pb = &B(0,0);
  int* pc = &C(0,0);
  
  //#pragma omp parallel for
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    A(r, c) = B(r, c) + C(r, c);
    //pa[r * A.ncols() + c] = pb[r * B.ncols() + c] + pc[r * C.ncols() + c];
    // equivalent to pa[r * A.ncols() + c] = pb[r * B.ncols() + c] + pc[r * C.ncols() + c];
  }
}


void raw_naive2d(int** pa, int** pb, int** pc, box2d A)
{
  const int nr = A.nrows();
  const int nc = A.ncols();
  // #pragma omp parallel for
  for (int r = 0; r < nr; r++)
    //#pragma omp simd
  for (int c = 0; c < nc; c++)
  {
    //A(r, c) = B(r, c) + C(r, c);
    pa[r][c] = pb[r][c] + pc[r][c];
    //pa[r * A.ncols() + c] = pb[r * B.ncols() + c] + pc[r * C.ncols() + c];
    // equivalent to pa[r * A.ncols() + c] = pb[r * B.ncols() + c] + pc[r * C.ncols() + c];
  }
}

template <typename I>
void check(I A, I B, I C)
{
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    if (A(r, c) != B(r, c) + C(r, c))
    {
      std::cout << A(r, c) << " " <<  B(r, c) << " " << C(r, c) << std::endl;
      throw std::runtime_error("error.");
    }
  }
}

template <typename I>
void mln_check(I A, I B, I C)
{
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    if (A(mln::point2d(r, c)) != B(mln::point2d(r, c)) + C(mln::point2d(r, c)))
      throw std::runtime_error("error.");
  }
}

//__attribute__((optimize("no-tree-vectorize")))
void raw_openmp_simd(image2d<int> A, image2d<int> B, image2d<int> C)
{
  const int nr = A.nrows();
  const int nc = A.ncols();

  //#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int* curB = &B(vint2(r, 0));
    int* curC = &C(vint2(r, 0));
    int* endA = curA + A.nrows();

    
    // while (curA != endA)
    //   {
    //     *curA = *curB + *curC;
    //     ++curA;
    //     ++curB;
    //     ++curC;
    //   }
    //int i;
    #pragma omp simd aligned(curA, curB, curC : 8 * sizeof(int))
    for (int i = 0; i < nc; i++)
     {
       curA[i] = curB[i] + curC[i];
    //   // curA[i+1] = curB[i+1] + curC[i+1];
    //   // curA[i+2] = curB[i+2] + curC[i+2];
    //   // curA[i+3] = curB[i+3] + curC[i+3];
    //   // curA[i+4] = curB[i+4] + curC[i+4];
    //   // curA[i+5] = curB[i+5] + curC[i+5];
    //   // curA[i+6] = curB[i+6] + curC[i+6];
    //   // curA[i+7] = curB[i+7] + curC[i+7];
     }
  }
}

void opencv(image2d<int>& A, image2d<int> B, image2d<int> C)
{
  cv::add(to_opencv(C), to_opencv(B), to_opencv(A));
}

void vpp_pixel_wise(image2d<int> A, image2d<int> B, image2d<int> C)
{
  vpp::pixel_wise(A, B, C) | [] (int& a, int& b, int& c)
  {
    a = b + c;
  };
}

static void BM_pixel_wise(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x());
  image2d<int> B(state.range_x(), state.range_x());
  image2d<int> C(state.range_x(), state.range_x());

  fill(A, 1);
  fill(B, 2);
  fill(C, 3);
  while (state.KeepRunning())
  {
      vpp_pixel_wise(A, B, C);
  }
  check(A, B, C);
}

static void BM_opencv(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x());
  image2d<int> B(state.range_x(), state.range_x());
  image2d<int> C(state.range_x(), state.range_x());

  fill(A, 1);
  fill(B, 2);
  fill(C, 3);
  while (state.KeepRunning())
      opencv(A, B, C);
  check(A, B, C);
}


static void BM_raw_naive2d(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x());
  image2d<int> B(state.range_x(), state.range_x());
  image2d<int> C(state.range_x(), state.range_x());

  fill(A, 1);
  fill(B, 2);
  fill(C, 3);

  int* pa[A.nrows()];
  int* pb[A.nrows()];
  int* pc[A.nrows()];

  for (int r = 0; r < A.nrows(); r++)
    {
      pa[r] = &A(r, 0);
      pb[r] = &B(r, 0);
      pc[r] = &C(r, 0);
    }
  
  while (state.KeepRunning())
    raw_naive2d(pa, pb, pc, A.domain());
  check(A, B, C);
}

static void BM_raw_naive(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x());
  image2d<int> B(state.range_x(), state.range_x());
  image2d<int> C(state.range_x(), state.range_x());

  fill(A, 1);
  fill(B, 2);
  fill(C, 3);
  while (state.KeepRunning())
  {
      raw_naive(A, B, C);
  }
  check(A, B, C);
}

static void BM_raw_openmp_simd(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x());
  image2d<int> B(state.range_x(), state.range_x());
  image2d<int> C(state.range_x(), state.range_x());

  fill(A, 1);
  fill(B, 2);
  fill(C, 3);
  while (state.KeepRunning())
  {
      raw_openmp_simd(A, B, C);
  }
  check(A, B, C);
}

static void pixter2d_add(benchmark::State& state)
{
  using namespace mln;
  mln::box2d b(point2d(0, 0), point2d(state.range_x(), state.range_x()));
  mln::image2d<int> A(b);
  mln::image2d<int> B(b);
  mln::image2d<int> C(b);

  mln_pixter_(mln::image2d<int>) p1(A);
  mln_pixter_(mln::image2d<int>) p2(B);
  mln_pixter_(mln::image2d<int>) p3(C);

  while (state.KeepRunning())
    {
    for_all_3(p1, p2, p3)
      p1.val() = p2.val() + p3.val();
    }
  mln_check(A, B, C);
}

static void piter2d_add(benchmark::State& state)
{
  using namespace mln;
  mln::box2d b(point2d(0, 0), point2d(state.range_x(), state.range_x()));
  mln::image2d<int> A(b);
  mln::image2d<int> B(b);
  mln::image2d<int> C(b);

  mln_piter_(mln::image2d<int>) p(A.domain());

  while (state.KeepRunning())
    {
      for_all(p)
        A(p) = B(p) + C(p);
    }
  mln_check(A, B, C);
}


BENCHMARK(BM_opencv)->Range(300, 2000);
BENCHMARK(BM_raw_naive)->Range(300, 2000);
BENCHMARK(BM_raw_naive2d)->Range(300, 2000);
BENCHMARK(BM_raw_openmp_simd)->Range(300, 2000);
BENCHMARK(BM_pixel_wise)->Range(300, 2000);
BENCHMARK(pixter2d_add)->Range(300, 2000);
BENCHMARK(piter2d_add)->Range(300, 2000);

BENCHMARK_MAIN();
