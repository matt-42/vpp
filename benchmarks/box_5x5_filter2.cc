#include <benchmark/benchmark.h>
#include <iostream>
#include <fstream>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <opencv2/opencv.hpp>

#include "get_time.hh"

using namespace vpp;


void raw_naive(image2d<int> A, image2d<int> B)
{
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    int sum = 0;
    for (int d = -2; d <= 2; d++)
    for (int e = -2; e <= 2; e++)
      sum += B(r + d, c + e);
    A(r, c) = sum / 25;
  }
}

void check(image2d<int> A, image2d<int> B)
{
  for (int r = 5; r < A.nrows() - 5; r++)
  for (int c = 5; c < A.ncols() - 5; c++)
  {

    int sum = 0;
    for (int d = -2; d <= 2; d++)
      for (int e = -2; e <= 2; e++)
        sum += B(r + d, c + e);

    if (A(r, c) != sum / 25)
      throw std::runtime_error("error!");
    assert(A(r, c) == sum / 25);
  }
}


void raw_openmp_simd(image2d<int> A, image2d<int> B)
{
  int nr = A.nrows();

#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int nc = A.ncols();

    int* rows[5];
    for (int i = -2; i <= 2; i++)
      rows[i + 2] = &B(vint2(r + i, 0));

    #pragma omp simd
    for (int c = 0; c < nc; c++)
    {
      int sum = 0;
      #pragma unroll
      for (int d = -2; d <= 2; d++)
        for (int e = -2; e <= 2; e++)
          sum += rows[d + 2][c + e];
      curA[c] = sum / 25;
    }
  }
}

void vpp_pixel_wise(image2d<int> B, image2d<int> A)
{
  vpp::pixel_wise(B, relative_access(A)) | [&] (int& b, auto a)
  {
    int sum = 0;
    for (int i = -2; i <= 2; i++)
    for (int j = -2; j <= 2; j++)
      sum += a(i, j);
    b = sum / 25;
  };
}

void opencv(image2d<int> A, image2d<int> B)
{
  cv::boxFilter(to_opencv(B), to_opencv(A), -1, cv::Size(5,5), cv::Point(-1, -1), true, cv::BORDER_CONSTANT);
}


static void BM_pixel_wise(benchmark::State& state)
{
  image2d<int> A(state.range(0), state.range(0), _border = 2);
  image2d<int> B(state.range(0), state.range(0), _border = 2);

  fill(A, 1);
  fill(B, 2);
  while (state.KeepRunning())
  {
      vpp_pixel_wise(A, B);
  }
}

static void BM_opencv(benchmark::State& state)
{
  image2d<int> A(state.range(0), state.range(0), _border = 2);
  image2d<int> B(state.range(0), state.range(0), _border = 2);

  fill(A, 1);
  fill(B, 2);
  while (state.KeepRunning())
      opencv(A, B);
}

static void BM_raw_openmp_simd(benchmark::State& state)
{
  image2d<int> A(state.range(0), state.range(0), _border = 2);
  image2d<int> B(state.range(0), state.range(0), _border = 2);
  fill(A, 1); fill(B, 2);
  while (state.KeepRunning())
    raw_openmp_simd(A, B);
}

BENCHMARK(BM_pixel_wise)->Range(100, 2000);
BENCHMARK(BM_raw_openmp_simd)->Range(100, 2000);
BENCHMARK(BM_opencv)->Range(100, 2000);

BENCHMARK_MAIN();
