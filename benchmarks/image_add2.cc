
#include <benchmark/benchmark.h>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <opencv2/opencv.hpp>

using namespace vpp;


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

void raw_openmp_simd(image2d<int> A, image2d<int> B, image2d<int> C)
{
  const int nr = A.nrows();
  const int nc = A.ncols();

  #pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int* curB = &B(vint2(r, 0));
    int* curC = &C(vint2(r, 0));
    int* endA = curA + A.nrows();
    
 #pragma omp simd aligned(curA, curB, curC : 8 * sizeof(int))
    for (int i = 0; i < nc; i++)
      curA[i] = curB[i] + curC[i];
  }
}

void opencv(image2d<int>& A, image2d<int> B, image2d<int> C)
{
  cv::add(to_opencv(C), to_opencv(B), to_opencv(A));
}

void vpp_pixel_wise(image2d<int> A,
                    image2d<int> B,
                    image2d<int> C)
{
  vpp::pixel_wise(A, B, C) | [] (int& a, int b, int c)
  {
    a = b + c;
  };
}


static void BM_pixel_wise(benchmark::State& state)
{
  image2d<int> A(state.range(0), state.range(0));
  image2d<int> B(state.range(0), state.range(0));
  image2d<int> C(state.range(0), state.range(0));

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
  image2d<int> A(state.range(0), state.range(0));
  image2d<int> B(state.range(0), state.range(0));
  image2d<int> C(state.range(0), state.range(0));

  fill(A, 1);
  fill(B, 2);
  fill(C, 3);
  while (state.KeepRunning())
      opencv(A, B, C);
  check(A, B, C);
}


static void BM_raw_openmp_simd(benchmark::State& state)
{
  image2d<int> A(state.range(0), state.range(0));
  image2d<int> B(state.range(0), state.range(0));
  image2d<int> C(state.range(0), state.range(0));

  fill(A, 1);
  fill(B, 2);
  fill(C, 3);
  while (state.KeepRunning())
  {
      raw_openmp_simd(A, B, C);
  }
  check(A, B, C);
}

BENCHMARK(BM_pixel_wise)->Range(300, 2000);
BENCHMARK(BM_opencv)->Range(300, 2000);
BENCHMARK(BM_raw_openmp_simd)->Range(300, 2000);

BENCHMARK_MAIN();
