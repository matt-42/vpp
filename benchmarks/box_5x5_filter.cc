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
#pragma omp parallel for
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

  std::cout << pitch << std::endl;
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

template <typename T>
int bench(int size, T& results, int debug = 0)
{
  std::cout << size << std::endl;
  int w = sqrt(size);
  image2d<int> A(w,w);
  image2d<int> B(A.domain(), _Border = 2);

  fill(A, 0);

  pixel_wise(B) | [] (int& b) { b = rand() % 1000; };

  int K = 10;
  double time;

  // Cache warm up.
  raw_naive(A, B);

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_naive(A, B);
  double raw_naive_time = get_time_in_seconds() - time;
  check(A, B);

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_openmp_simd(A, B);
  double raw_openmp_simd_time = get_time_in_seconds() - time;
  check(A, B);


  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_openmp_simd2(A, B);
  double raw_openmp_simd2_time = get_time_in_seconds() - time;
  check(A, B);

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    vpp_pixel_wise(A, B);
  double pixel_wise_time = get_time_in_seconds() - time;
  check(A, B);


  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    opencv(A, B);
  double opencv_time = get_time_in_seconds() - time;
  check(A, B);

  double freq = 3.2 * 1000 * 1000 * 1000;
  if (debug)
  {
    std::cout << "time per iteration (ms) : " << std::endl;
    std::cout << "raw_naive_time: " << 1000. * raw_naive_time / K << std::endl;
    std::cout << "raw_openmp_simd_time: " << 1000. * raw_openmp_simd_time / K << std::endl;
    std::cout << "raw_openmp_simd2_time: " << 1000. * raw_openmp_simd2_time / K << std::endl;
    std::cout << "vpp_pixel_wise: " << 1000. * pixel_wise_time / K << std::endl;

    double c = freq / (K * size);
    std::cout << "raw_naive_time: " << c * raw_naive_time << std::endl;
    std::cout << "raw_openmp_simd_time: " << c * raw_openmp_simd_time << std::endl;
    std::cout << "raw_openmp_simd2_time: " << c * raw_openmp_simd2_time << std::endl;
    std::cout << "vpp_pixel_wise: " << c * pixel_wise_time << std::endl;
  }
  results.naive.push_back(freq * raw_naive_time / (K * size));
  results.pixel_wise.push_back(freq * pixel_wise_time / (K * size));
  results.raw.push_back(freq * raw_openmp_simd_time / (K * size));
  results.opencv.push_back(freq * opencv_time / (K * size));
}

int main()
{

  ::sched_param sc_params;
  sc_params.sched_priority = 1;
  if (::sched_setscheduler(::getpid(), SCHED_FIFO, &sc_params))
    ::fprintf(stderr, "sched_setscheduler(): %s\n", ::strerror(errno));

  struct {
    std::vector<double> naive;
    std::vector<double> pixel_wise;
    std::vector<double> raw;
    std::vector<double> opencv;
  } results;

  int step = 100000;
  int max_size = 2000 * 2000;
  // for (int s = step; s < max_size; s += step)
  //   bench(s, results, 1);
  bench(4e6, results, 1);
  std::ofstream n("box_5x5_naive.txt");
  std::ofstream p("box_5x5_pixel_wise.txt");
  std::ofstream o("box_5x5_openmp.txt");
  std::ofstream ocv("box_5x5_opencv.txt");
  std::ofstream speedup("box_5x5_speedup.txt");
  for (int s = 0; s < (max_size/step - 1); s ++)
  {
    n << (s+1) * step << '\t'<< results.naive[s] << std::endl;
    p << (s+1) * step << '\t'<< results.pixel_wise[s] << std::endl;
    o << (s+1) * step << '\t'<< results.raw[s] << std::endl;
    ocv << (s+1) * step << '\t'<< results.opencv[s] << std::endl;
    speedup << (s+1) * step << '\t'<< results.naive[s] / results.pixel_wise[s] << std::endl;
  }
}
