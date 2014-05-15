#include <iostream>
#include <fstream>
#include <vpp/boxNd.hh>
#include <vpp/vpp.hh>

#include "get_time.hh"

using namespace vpp;


void raw_naive(image2d<int> A, image2d<int> B)
{
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    int sum = 0;
    for (int d = -2; d <= 2; d++)
      sum += B(r, c + d);
    A(r, c) = sum / 5;
  }
}

void check(image2d<int> A, image2d<int> B)
{
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    assert(A(r, c) == (B(r, c - 2) + B(r, c - 1) + B(r, c) + B(r, c + 1) + B(r, c + 2)) / 5);
  }
}


void raw_openmp_simd(image2d<int> A, image2d<int> B)
{
  int nr = A.nrows();
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int* curB = &B(vint2(r, 0));
    int* endA = curA + A.nrows();

    int nc = A.ncols();
    for (int i = 0; i < nc; i++)
    {
      int sum = 0;
      for (int d = -2; d <= 2; d++)
        sum += curB[i + d];
      curA[i] = sum / 5;
    }
  }
}

void vpp_pixel_wise(image2d<int> A, image2d<int> B)
{
  auto nbh = box_nbh2d<int, 1, 5>(B);
  vpp::pixel_wise(A, B) << [&] (int& a, int& b)
  {
    int sum = 0;
    nbh(b) << [&] (int& n) { sum += n; };
    a = sum / 5;
  };
}

template <typename T>
int bench(int size, T& results, int debug = 0)
{
  std::cout << size << std::endl;
  image2d<int> A(sqrt(size),sqrt(size));
  image2d<int> B(A.domain(), 2);

  fill(A, 0);

  pixel_wise(B) << [] (int& b) { b = rand() % 1000; };

  int K = 40;
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
    vpp_pixel_wise(A, B);
  double pixel_wise_time = get_time_in_seconds() - time;
  check(A, B);

  double freq = 2.5 * 1000 * 1000 * 1000;
  if (debug)
  {
    std::cout << "time per iteration (ms) : " << std::endl;
    std::cout << "raw_naive_time: " << 1000. * raw_naive_time / K << std::endl;
    std::cout << "raw_openmp_simd_time: " << 1000. * raw_openmp_simd_time / K << std::endl;
    std::cout << "vpp_pixel_wise: " << 1000. * pixel_wise_time / K << std::endl;

    double c = freq / (K * size);
    std::cout << "raw_naive_time: " << c * raw_naive_time << std::endl;
    std::cout << "raw_openmp_simd_time: " << c * raw_openmp_simd_time << std::endl;
    std::cout << "vpp_pixel_wise: " << c * pixel_wise_time << std::endl;
  }
  results.naive.push_back(freq * raw_naive_time / (K * size));
  results.pixel_wise.push_back(freq * pixel_wise_time / (K * size));
  results.raw.push_back(freq * raw_openmp_simd_time / (K * size));
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
  } results;

  int step = 10000;
  int max_size = 1000 * 1000;
  for (int s = step; s < max_size; s += step)
    bench(s, results);

  std::ofstream n("box_naive.txt");
  std::ofstream p("box_pixel_wise.txt");
  std::ofstream o("box_openmp.txt");
  std::ofstream speedup("box_speedup.txt");
  for (int s = 0; s < max_size/step; s ++)
  {
    n << s * step << '\t'<< results.naive[s] << std::endl;
    p << s * step << '\t'<< results.pixel_wise[s] << std::endl;
    o << s * step << '\t'<< results.raw[s] << std::endl;
    speedup << s * step << '\t'<< results.naive[s] / results.pixel_wise[s] << std::endl;
  }
}
