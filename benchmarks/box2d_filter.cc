#include <iostream>
#include <fstream>
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

typedef unsigned int uint32;

uint32 interleave_with_zeros(uint32 x)
{
  x &= 0x0000ffff;
  x = (x ^ (x <<  8)) & 0x00ff00ff;
  x = (x ^ (x <<  4)) & 0x0f0f0f0f;
  x = (x ^ (x <<  2)) & 0x33333333;
  x = (x ^ (x <<  1)) & 0x55555555;
  return x;
}


uint32 morton_compact(uint32 x)
{
  x &= 0x55555555;                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  x = (x ^ (x >>  1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x >>  2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x >>  4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x >>  8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
  return x;
}

uint32 morton_encode2d(uint32 x, uint32 y)
{
  return (interleave_with_zeros(x)) | (interleave_with_zeros(y) << 1); 
}

void morton_decode2d(uint32 i, uint32* x, uint32* y)
{
  *x = morton_compact(i);
  *y = morton_compact(i >> 1);
}

void z_order(image2d<int> A, image2d<int> B, int W)
{
  int nr = A.nrows();
  int nc = A.ncols();
  int n_pixels = A.nrows() * A.ncols();

  int* data = &A(0,0);
  const int* end = A.data_end();
  int* B_data = &B(0,0);

#pragma omp parallel for schedule(static, 1000)
  for (int pi = 0; pi < n_pixels; pi++)
  {
    unsigned int r_, c_;
    morton_decode2d(pi, &r_, &c_);

    int r = r_;
    int c = c_;

    int sum = 0;
    if (r > W && r < nr - W &&
        c > W && c < nc - W)
    {
      for (int nr = -W; nr <= W; nr++)
//#pragma omp simd
        for (int nc = -W; nc <= W; nc++)
        {
          //int n = morton_encode2d((r + nr) / 16, (c + nc) / 16);
          const int TW = 1;
          // int n = morton_encode2d((r + nr) / TW, (c + nc) / TW) * (TW * TW) +
          //   ((r + nr) % TW) * TW + ((c + nc) % TW);

          //if (data + n < end)
          {
            //std::cout << n << ":" << data[n] << std::endl;
            sum += data[n];
          }
          // else
          // {
          //   std::cout << "error at " << r + nr << ":" << c + nc << ": " << n << " > " << size_t(end - data) << std::endl;
          // }
        }
    }
    if (sum != 0)
      std::cout << r << " " << c << ": " << sum << std::endl;
    B_data[pi] = sum;
  }
}


void z_order(image2d<int> A, image2d<int> B, std::vector<vint2>& points, int W)
{
  int nr = A.nrows();
  int nc = A.ncols();
  int n_pixels = A.nrows() * A.ncols();

  int* data = &A(0,0);
  const int* end = A.data_end();
  int* B_data = &B(0,0);

#pragma omp parallel for
  for (int pi = 0; pi < points.size(); pi++)
  {
    int r = points[pi][0];
    int c = points[pi][1];

    int* data = &A(r,c);

    int sum = 0;
    if (r > W && r < nr - W &&
        c > W && c < nc - W)
    {
      for (int nr = -W; nr <= W; nr++)
      {
        int* row = data + nr * 128;
//#pragma omp simd
        for (int nc = -W; nc <= W; nc++)
        {
          //int n = morton_encode2d((r + nr), (c + nc));
          //const int TW = 64;
          // int n = morton_encode2d((r + nr) / TW, (c + nc) / TW) * (TW * TW) +
          //   ((r + nr) % TW) * TW + ((c + nc) % TW);

          //if (data + n < end)
          {
            //std::cout << n << ":" << data[n] << std::endl;
            row[nc] = 1;
          }
          // else
          // {
          //   std::cout << "error at " << r + nr << ":" << c + nc << ": " << n << " > " << size_t(end - data) << std::endl;
          // }
        }
      }
    }
    if (sum != 0)
      std::cout << r << " " << c << ": " << sum << std::endl;
    B_data[pi] = sum;
  }
}


void raw_order(image2d<int> A, image2d<int> B, int W)
{
  int nr = A.nrows();
  int nc = A.ncols();
  int n_pixels = A.nrows() * A.ncols();

#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* rows[W * 2 + 1];
    int* Brow = &B(r, 0);
    if (r <= W || r >= nr - W) continue;

    for (int nr = -W; nr <= W; nr++)
      rows[nr + W] = &A(r + nr, 0);

    for (int c = W; c < nc - W; c++)
    {
      int sum = 0;
      if (c > W && c < nc - W)
      {
        for (int nr = -W; nr <= W; nr++)
        {
          int* P = rows[nr + W] + c;
          for (int nc = -W; nc <= W; nc++)
          {
            //sum += P[nc];
            P[nc] = 1;
            //sum += A(r + nr, c + nc);
          }
        }
      }
      Brow[c] = sum;
    }
  }
}

void raw_order(image2d<int> A, image2d<int> B, std::vector<vint2>& points, int W)
{
  int nr = A.nrows();
  int nc = A.ncols();
  int n_pixels = A.nrows() * A.ncols();

#pragma omp parallel for
  for (int i = 0; i < points.size(); i++)
  {
    int r = points[i][0];
    int c = points[i][1];

    int* rows[W * 2 + 1];
    int* Brow = &B(r, c);
    if (r <= W || r >= nr - W) continue;

    for (int nr = -W; nr <= W; nr++)
      rows[nr + W] = &A(r + nr, c);

    int sum = 0;
    if (c > W && c < nc - W)
    {
      for (int nr = -W; nr <= W; nr++)
      {
        int* P = rows[nr + W] + c;
        for (int nc = -W; nc <= W; nc++)
        {
          sum += P[nc];
          //sum += A(r + nr, c + nc);
        }
      }
    }
    Brow[c] = sum;
  }
}

template <typename T>
int bench(int size, T& results, int debug = 0)
{
  std::cout << size << std::endl;
  //image2d<int> A(sqrt(size),sqrt(size), border(sqrt(size)));
  image2d<int> A(500, 500 * 500, border(300));
  image2d<int> B(A.domain(), border(sqrt(size)));

  std::cout << "fill(A, 0)" << std::endl;
  fill(A, 0);

  std::cout << "randomize" << std::endl;
  //pixel_wise(B) << [] (int& b) { b = rand() % 1000; };

  int K = 1;
  int W = 40;
  double time;

  std::cout << "generate points" << std::endl;

  std::vector<vint2> points;
  for (int i = 0; i < 10000; i++)
    points.push_back({rand() % A.nrows(), rand() % A.ncols()});

  // Cache warm up.
  raw_naive(A, B);

  std::cout << "z_order" << std::endl;

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    z_order(A, B, W);
  double z_order_time = get_time_in_seconds() - time;
  //check(A, B);

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_order(A, B, W);
  double raw_order_time = get_time_in_seconds() - time;
  //check(A, B);

  double freq = 3.7 * 1000 * 1000 * 1000;
  if (debug)
  {
    std::cout << "time per iteration (ms) : " << std::endl;
    std::cout << "z_order: " << 1000. * z_order_time / K << std::endl;
    std::cout << "raw_order: " << 1000. * raw_order_time / K << std::endl;

    double c = freq / (K * size);
    std::cout << "z_order_time: " << c * z_order_time << std::endl;
    std::cout << "raw_order_time: " << c * raw_order_time << std::endl;
  }
  // results.naive.push_back(freq * raw_naive_time / (K * size));
  // results.pixel_wise.push_back(freq * pixel_wise_time / (K * size));
  // results.raw.push_back(freq * raw_openmp_simd_time / (K * size));
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

  bench(500, results, 1);

  // int step = 10000;
  // int max_size = 1000 * 1000;
  // for (int s = step; s < max_size; s += step)
  //   bench(s, results);

  // std::ofstream n("box_naive.txt");
  // std::ofstream p("box_pixel_wise.txt");
  // std::ofstream o("box_openmp.txt");
  // std::ofstream speedup("box_speedup.txt");
  // for (int s = 0; s < max_size/step; s ++)
  // {
  //   n << s * step << '\t'<< results.naive[s] << std::endl;
  //   p << s * step << '\t'<< results.pixel_wise[s] << std::endl;
  //   o << s * step << '\t'<< results.raw[s] << std::endl;
  //   speedup << s * step << '\t'<< results.naive[s] / results.pixel_wise[s] << std::endl;
  // }
}
