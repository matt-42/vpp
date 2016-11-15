#include <chrono>
#include <bitset>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
//#include <vpp/algorithms/FAST_detector/FAST.hh>

#include "get_time.hh"

using namespace vpp;
template <typename V>
void test1(image2d<V>& A, image2d<V>& B)
{
  pixel_wise(A, B) | [&] (unsigned char& a, unsigned char& b)
  {
    b = a;
  };
}

template <typename V>
V* shift(V* p, int diff)
{
  return (V*)(((char*)p) + diff);
}

template <typename V>
void test2(image2d<V>& A, image2d<V>& B)
{
  int nc = A.ncols();
  int nr = A.nrows();
  int pitch = A.pitch();
  //row_wise(A, B) << [&] (unsigned char& a, unsigned char& b)
    // V* a_row = &a;
    // V* b_row = &b;
  #pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    V* a_row = &A(r, 0);
    V* b_row = &B(r, 0);

    #pragma omp simd
    for (int c = 0; c < nc; c++)
    {
      b_row[c] = a_row[c];
    }
  }
}

template <typename V>
void test3(image2d<V>& A, image2d<int>& B)
{
  int nc = A.ncols();
  int nr = A.nrows();
  int pitch = A.pitch();
  //row_wise(A, B) << [&] (unsigned char& a, unsigned char& b)
    // V* a_row = &a;
    // V* b_row = &b;
  #pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    V* a_row1 = &A(r, 0);
    V* a_row2 = shift(a_row1, 3*pitch);
    V* a_row3 = shift(a_row1, 2*pitch);
    V* a_row4 = shift(a_row1, 1*pitch);
    V* a_row5 = shift(a_row1, -1*pitch);
    V* a_row6 = shift(a_row1, -2*pitch);
    V* a_row7 = shift(a_row1, -3*pitch);

    auto* b_row = &B(r, 0);

    int th = 10;
#pragma omp simd aligned(a_row1, a_row2, a_row3, a_row4, a_row5, a_row6, a_row7, b_row:16 * sizeof(int))
    for (int c = 0; c < nc; c++)
    {
    V v = a_row1[c];

    auto f = [&] (V a) -> int { return (a > v + th) ? 2 : (a < v - th); };

    unsigned int x = 0;
    x =
      f(a_row2[c - 1]) +
      (f(a_row2[c]) << 2) +
      (f(a_row2[c + 1]) << 4) +

      (f(a_row3[c + 2]) << 6) +

      (f(a_row4[c + 3]) << 8) +
      (f(a_row1[c + 3]) << 10) +
      (f(a_row5[c + 3]) << 12) +

      (f(a_row6[c + 2]) << 14) +

      (f(a_row7[c + 1]) << 16) +
      (f(a_row7[c]) << 18) +
      (f(a_row7[c - 1]) << 20) +

      (f(a_row6[c - 2]) << 22) +

      (f(a_row5[c - 3]) << 24) +
      (f(a_row1[c - 3]) << 26) +
      (f(a_row4[c - 3]) << 28) +

      (f(a_row3[c - 2]) << 30);

      b_row[c] = x;
    }
  }
}

#include "fast_count.hh"

void fast_count(image2d<int>& A, image2d<unsigned char>& B)
{
  int nc = A.ncols();
  int nr = A.nrows();
  int pitch = A.pitch();
  //row_wise(A, B) << [&] (unsigned char& a, unsigned char& b)
  // V* a_row = &a;
  // V* b_row = &b;
  #pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    auto* a_row = &A(r, 0);
    auto* b_row = &B(r, 0);

    #pragma omp simd
    for (int c = 0; c < nc; c++)
    {
      //b_row[c] = Find9Opt(a_row[c]);
      b_row[c] = Find9Jeremy(a_row[c]);
      //b_row[c] = fast_count_pixel3(a_row[c]);
  }
  }
}

inline uint rol2(uint code32)
{
  unsigned int x = code32;
  asm("rol $4, %0" : "+r" (code32));
  return code32;
}

int main(int argc, char* argv[])
{
 unsigned int x = 1;
 std::cout << std::bitset<32>(rol2(x)) << std::endl;
return 1;
// Find9Jeremy(0b00000001010101010101010101001011);
// return 1;
  ::sched_param sc_params;
  sc_params.sched_priority = 1;
  if (::sched_setscheduler(::getpid(), SCHED_FIFO, &sc_params))
    ::fprintf(stderr, "sched_setscheduler(): %s\n", ::strerror(errno));

  using namespace vpp;

  // int v = 1;
  // for (unsigned i = 0; i < 40; i++)
  // {
  //   std::cout << std::bitset<32>(v) << std::endl;
  //   v = rol2(v);
  // }
  // std::cout << std::bitset<32>(rol2(rol2(2))) << std::endl;

  // return 0;

  // std::cout << fast_count_pixel3(0b00010101010101010101000000000000) << std::endl;
  // return 0;

  {
    double freq = 3.7 * 1000 * 1000 * 1000;
    int N = 1000;

    std::vector<unsigned int> vec(N);
    std::vector<unsigned int> out(N);

    for (int i = 0; i < N; i++)
      vec[i] =  rand() & 0xff
        + (rand() & 0xff) << 8
        + (rand() & 0xff) << 16
        + (rand() & 0xff) << 24;

    auto time = get_time_in_seconds();

    int K = 1000;
    for (int k = 0; k < K; k++)
    {
    #pragma omp parallel for
      for (int i = 0; i < N; i++)
      {
    //out[i] = fast_count_pixel3(vec[i]);
        //out[i] = Find9Opt(vec[i]);
        out[i] = Find9Jeremy(vec[i]);
        // if (Find9Jeremy(vec[i]) != Find9Opt(vec[i]) || Find9Jeremy(vec[i]) != fast_count_pixel3(vec[i]))
        //   std::cout << "Error" << std::endl;
      }
    }
    double ms_per_iter = (get_time_in_seconds() - time);
    std::cout << "fast_count_pixel3: " << freq * ms_per_iter / (N* K) << " cpp " << out[10] << std::endl;
  }

  std::cout << fast_count_pixel3(0b10000000100000001000000010000000) << std::endl;
  std::cout << fast_count_pixel3(0b00000001010101010101010101001011) << std::endl;

  std::cout << Find9Opt(0b10000000100000001000000010000000) << std::endl;
  std::cout << Find9Opt(0b00000001010101010101010101001011) << std::endl;
  std::cout << Find9Opt(0b00000001010101010100110101001011) << std::endl;

  // return 0;

  if (argc != 2)
  {
    std::cerr << "Usage : " << argv[0] << " image" << std::endl;
    return 1;
  }

  int K = 200;

  typedef image2d<vuchar3> I;
  I A = clone(from_opencv<vuchar3>(cv::imread(argv[1])), _border = 3);
  I B(A.domain(), _border = 1);

  image2d<unsigned char> Agl(A.domain(), _border = 3);
  image2d<unsigned char> Bgl(A.domain());
  image2d<int> Bint(A.domain());
  image2d<unsigned char> kp(A.domain());

  pixel_wise(Agl, A) | [] (unsigned char& gl, vuchar3& c)
  {
    gl = (c[0] + c[1] + c[2]) / 3;
  };

  auto time = get_time_in_seconds();
  for (unsigned i = 0; i < K; i++)
    test1(Agl, Bgl);
  double test1_ms_per_iter = (get_time_in_seconds() - time);

  time = get_time_in_seconds();
  for (unsigned i = 0; i < K; i++)
    test2(Agl, Bgl);
  double test2_ms_per_iter = (get_time_in_seconds() - time);


  time = get_time_in_seconds();
  for (unsigned i = 0; i < K; i++)
  {
    test3(Agl, Bint);
    //fast_count(Bint, kp);
  }
  double test3_ms_per_iter = (get_time_in_seconds() - time);


  time = get_time_in_seconds();
  for (unsigned i = 0; i < K; i++)
    fast_count(Bint, kp);
  double test4_ms_per_iter = (get_time_in_seconds() - time);

  int sum = 0;
  pixel_wise(kp) | [&] (unsigned char& k)
  {
    sum += k;
  };
  std::cout << sum << " keypoints."<< std::endl;

  double freq = 3.7 * 1000 * 1000 * 1000;

  int size = A.nrows() * A.ncols();
  std::cout << "Time per iterations: " << std::endl
            << "test1: " << 1000* test1_ms_per_iter / K << "ms" << std::endl
            << "test2: " << 1000* test2_ms_per_iter / K << "ms" << std::endl
            << "test3: " << 1000* test3_ms_per_iter / K << "ms" << std::endl
            << "test4: " << 1000* test4_ms_per_iter / K << "ms" << std::endl
            << "test3: " << freq * test3_ms_per_iter / (K * size) << " cpp" << std::endl
            << "test4: " << freq * test4_ms_per_iter / (K * size) << " cpp" << std::endl;
}
