#include <iostream>
#include <vpp/vpp.hh>

#include "get_time.hh"

using namespace vpp;


void raw_naive(image2d<int> img)
{
  for (int r = 0; r < img.nrows(); r++)
  for (int c = 0; c < img.ncols(); c++)
    img(r, c) = r + c;
}

void check(image2d<int> img)
{
  for (int r = 0; r < img.nrows(); r++)
  for (int c = 0; c < img.ncols(); c++)
  {
    assert(img(r, c) == r + c);
  }
}

void raw_sequential(image2d<int> img)
{
  int nr = img.nrows();
  for (int r = 0; r < nr; r++)
  {
    int* cur = &img(vint2(r, 0));
    int* end = cur + img.nrows();
    int c = 0;
    while (cur != end)
    {
      *cur = r + c;
      ++cur;
      c++;
    }
  }
}

void raw_openmp(image2d<int> img)
{
  int nr = img.nrows();
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* cur = &img(vint2(r, 0));
    int* end = cur + img.ncols();
    int c = 0;
    while (cur != end)
    {
      *cur = r + c;
      ++cur;
      c++;
    }
  }
}


void raw_openmp_simd(image2d<int> img)
{
  int nr = img.nrows();
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* cur = &img(vint2(r, 0));
    int nc = img.ncols();

#pragma omp simd
    for (int i = 0; i < nc; i++)
    {
      cur[i] = r;
    }
  }
}

void vpp_pixel_wise(image2d<int> img)
{
  vpp::pixel_wise(img, img.domain()) | [] (auto& p, auto& c)
  {
    p = c[0] + c[1];
  };
}

int main()
{
  ::sched_param sc_params;
  sc_params.sched_priority = 1;
  if (::sched_setscheduler(::getpid(), SCHED_FIFO, &sc_params))
    ::fprintf(stderr, "sched_setscheduler(): %s\n", ::strerror(errno));

  image2d<int> img(1000,1000);

  int K = 400;
  double time;


  // Cache warm up.
  raw_naive(img);

  fill(img, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_naive(img);
  double raw_naive_time = get_time_in_seconds() - time;
  check(img);

  fill(img, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_sequential(img);
  double raw_sequential_time = get_time_in_seconds() - time;
  check(img);

  fill(img, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_openmp(img);
  double raw_openmp_time = get_time_in_seconds() - time;
  check(img);


  fill(img, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_openmp_simd(img);
  double raw_openmp_simd_time = get_time_in_seconds() - time;
  // check(img);

  fill(img, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    vpp_pixel_wise(img);
  double pixel_wise_time = get_time_in_seconds() - time;
  check(img);

  std::cout << "time per iteration (ms) : " << std::endl;
  std::cout << "raw_naive_time: " << 1000. * raw_naive_time / K << std::endl;
  std::cout << "raw_sequential_time: " << 1000. * raw_sequential_time / K << std::endl;
  std::cout << "raw_openmp_time: " << 1000. * raw_openmp_time / K << std::endl;
  std::cout << "raw_openmp_simd_time: " << 1000. * raw_openmp_simd_time / K << std::endl;
  std::cout << "vpp_pixel_wise: " << 1000. * pixel_wise_time / K << std::endl;
  //std::cout << "domain iteration overhead: " << 100. * id_time / ref_time - 100. << "%" << std::endl;

}
