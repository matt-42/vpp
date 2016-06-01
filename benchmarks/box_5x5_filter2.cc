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

    assert(A(r, c) == sum / 25);
  }
}


void raw_openmp_simd(image2d<int> A, image2d<int> B)
{
  int nr = A.nrows();

  // int* rows_[B.nrows() + B.border()];
  // for (int i = -B.border(); i < B.nrows() + B.border(); i++)
  //   rows_[i] = &B(i, 0);

  // int** rows = rows_ + B.border();

#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int nc = A.ncols();

    int* rows[5];
    for (int i = -2; i <= 2; i++)
      rows[i + 2] = &B(vint2(r + i, 0));

    //#pragma omp simd
    for (int i = 0; i < nc; i++)
    {
      int sum = 0;
      #pragma unroll
      for (int d = -2; d <= 2; d++)
        for (int e = -2; e <= 2; e++)
          //sum += rows[r + d][i + e];
          sum += rows[d + 2][i + e];
      curA[i] = sum / 25;
    }
  }
}

void raw_serial(image2d<int> A, image2d<int> B)
{
  int nr = A.nrows();

  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int nc = A.ncols();

    int* rows[5];
    for (int i = -2; i <= 2; i++)
      rows[i + 2] = &B(vint2(r + i, 0));

    for (int i = 0; i < nc; i++)
    {
      int sum = 0;
      for (int d = -2; d <= 2; d++)
        for (int e = -2; e <= 2; e++)
          sum += rows[d + 2][i + e];
      curA[i] = sum / 25;
    }
  }
}


// void raw_openmp_simd2(image2d<int> A, image2d<int> B)
// {
//   int nr = A.nrows();
// #pragma omp parallel for
//   for (int r = 0; r < nr; r++)
//   {
//     int* curA = &A(vint2(r, 0));
//     int nc = A.ncols();

//     int* rows[5];
//     for (int i = -2; i <= 2; i++)
//       rows[i + 2] = &B(vint2(r + i, 0));

//     for (int i = 0; i < nc; i++)
//     {
//       int sum = curA[i-1];

//       for (int d = -2; d <= 2; d++)
//         sum -= rows[d + 2][i - 3];
//       for (int d = -2; d <= 2; d++)
//         sum += rows[d + 2][i + 2];
//       curA[i] = sum;
//     }
//   }
// }


void raw_openmp_simd2(image2d<int> A, image2d<int> B)
{
  int nr = A.nrows();
  int pitch = B.pitch();

  //  std::cout << pitch << std::endl;
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int* curB = &B(vint2(r, 0));
    int nc = A.ncols();


    for (int i = 0; i < nc; i++)
    {
      int sum = curA[i-1];

      for (int d = -2; d <= 2; d++)
      {
        for (int e = -2; e <= 2; e++)
          sum += *(const int*)(((const char*) curB)  + (d * pitch) + (e + i) * sizeof(int));

      }

      // for (int e = -2; e <= 2; e++)
      //   sum += *(int*)((char*) curB + (e + i) * sizeof(int) + -2 * 8064);
      // for (int e = -2; e <= 2; e++)
      //   sum += *(int*)((char*) curB + (e + i) * sizeof(int) + -1 * 8064);
      // for (int e = -2; e <= 2; e++)
      //   sum += *(int*)((char*) curB + (e + i) * sizeof(int) + -0 * 8064);
      // for (int e = -2; e <= 2; e++)
      //   sum += *(int*)((char*) curB + (e + i) * sizeof(int) + 1 * 8064);
      // for (int e = -2; e <= 2; e++)
      //   sum += *(int*)((char*) curB + (e + i) * sizeof(int) + 2 * 8064);
      // for (int e = -2; e <= 2; e++)
      //   sum += *(int*)((char*) curB + (e + i) * sizeof(int) + 3 * 8064);
      
      curA[i] = sum;
    }
  }
}

void raw_openmp_simd3(image2d<int> A, image2d<int> B)
{
  int nr = A.nrows();
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int nc = A.ncols();

    vint8* rows[5];
    for (int i = -2; i <= 2; i++)
      rows[i + 2] = (vint8*)&B(vint2(r + i, 0));

    vector<int, 24> sums = vector<int, 24>::Zero();
    for (int i = 0; i < (nc / 8) - 1; i++)
    {
      sums.segment<8>(16) = rows[0][i] + rows[1][i] + rows[2][i] + rows[3][i] + rows[4][i];

      curA[i * 8 + 0] = sums.segment<5>(6).sum() / 25;
      for (unsigned j = 1; j < 8; j++)
        curA[i * 8 + j] = curA[i * 8 + j - 1] + (sums[6 + 5 + j] - sums[6 - 1 + j]) / 25;
      sums = vector<int, 24>::Zero();
      sums.segment<16>(0) = sums.segment<16>(8);
    }
  }
}

void vpp_pixel_wise(image2d<int> B, image2d<int> A)
{
  auto Anbh = box_nbh2d<int, 5, 5>(A);
  vpp::pixel_wise(B, Anbh) | [&] (int& b, auto& a_nbh)
  {
    int sum = 0;
    a_nbh.for_all([&sum] (int& n) { sum += n; });
    b = sum / 25;
  };
}

void opencv(image2d<int> A, image2d<int> B)
{
  cv::boxFilter(to_opencv(B), to_opencv(A), -1, cv::Size(5,5), cv::Point(-1, -1), true, cv::BORDER_CONSTANT);
  //cv::blur(to_opencv(B), to_opencv(A), cv::Size(5,5), cv::Point(-1, -1), true, cv::BORDER_CONSTANT);
}


static void BM_pixel_wise(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x(), _border = 2);
  image2d<int> B(state.range_x(), state.range_x(), _border = 2);

  fill(A, 1);
  fill(B, 2);
  while (state.KeepRunning())
  {
      vpp_pixel_wise(A, B);
  }
}

static void BM_opencv(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x(), _border = 2);
  image2d<int> B(state.range_x(), state.range_x(), _border = 2);

  fill(A, 1);
  fill(B, 2);
  while (state.KeepRunning())
      opencv(A, B);
}

static void BM_raw_naive(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x(), _border = 2);
  image2d<int> B(state.range_x(), state.range_x(), _border = 2);

  fill(A, 1);
  fill(B, 2);
  while (state.KeepRunning())
  {
      raw_naive(A, B);
  }
}


static void BM_raw_openmp_simd(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x(), _border = 2);
  image2d<int> B(state.range_x(), state.range_x(), _border = 2);
  fill(A, 1); fill(B, 2);
  while (state.KeepRunning())
    raw_openmp_simd(A, B);
}

static void BM_raw_serial(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x(), _border = 2);
  image2d<int> B(state.range_x(), state.range_x(), _border = 2);
  fill(A, 1); fill(B, 2);
  while (state.KeepRunning())
    raw_serial(A, B);
}


static void BM_raw_openmp_simd2(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x(), _border = 2);
  image2d<int> B(state.range_x(), state.range_x(), _border = 2);
  fill(A, 1); fill(B, 2);
  while (state.KeepRunning())
    raw_openmp_simd2(A, B);
}

static void BM_raw_openmp_simd3(benchmark::State& state)
{
  image2d<int> A(state.range_x(), state.range_x(), _border = 2);
  image2d<int> B(state.range_x(), state.range_x(), _border = 2);
  fill(A, 1); fill(B, 2);
  while (state.KeepRunning())
    raw_openmp_simd3(A, B);
}

BENCHMARK(BM_raw_openmp_simd)->Range(100, 2000);
BENCHMARK(BM_raw_serial)->Range(100, 2000);

BENCHMARK(BM_pixel_wise)->Range(100, 2000);
BENCHMARK(BM_opencv)->Range(100, 2000);
BENCHMARK(BM_raw_naive)->Range(100, 2000);

BENCHMARK_MAIN();
