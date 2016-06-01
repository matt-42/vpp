#include <benchmark/benchmark.h>

#include <chrono>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/fast_detector/fast.hh>

using namespace vpp;
typedef image2d<vuchar3> I;

static image2d<unsigned char> init_input(benchmark::State& state)
{
  image2d<unsigned char> A(state.range_x(), state.range_x(), _border = 3);
  I src = clone(from_opencv<vuchar3>(cv::imread("./img.jpg")), _border = 3);
  image2d<unsigned char> src_gl = rgb_to_graylevel<unsigned char>(src);
  
  for(vint2 p : A.domain())
    A(p) = src_gl(p[0] % std::min(A.nrows(), src.nrows()),
                  p[1] % std::min(A.ncols(), src.ncols()));

  fill_border_mirror(A);
  return A;
}

static void vpp_raw(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);
  std::vector<vint2> keypoints;

  const int th = state.range_y();
  
  while (state.KeepRunning())
  {
    keypoints.clear();
    keypoints = fast_detector9(A, th);
  }
  //std::cout << "vpp raw " << keypoints.size() << std::endl;
  
}


static void vpp_raw_lm(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);
  std::vector<vint2> keypoints;
  const int th = state.range_y();
  while (state.KeepRunning())
  {
    keypoints.clear();
    keypoints = fast_detector9_local_maxima(A, th);
  }
  //std::cout << "vpp lm " << keypoints.size() << std::endl;
}


static void vpp_raw_bm(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);
  std::vector<vint2> keypoints;
  const int th = state.range_y();
  while (state.KeepRunning())
  {
    keypoints.clear();
    keypoints = fast_detector9_blockwise_maxima(A, th, 10);
  }
  // //std::cout << "vpp bm " << keypoints.size() << std::endl;
}


static void vpp_opencv_lm(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);

  cv::Mat cv_A = to_opencv(A);

  const int th = state.range_y();

  std::vector<cv::KeyPoint> keypoints;
  while (state.KeepRunning())
  {
    keypoints.clear();
    FAST(cv_A, keypoints, th, true);
  }
  //std::cout << "opencv lm " << keypoints.size() << std::endl;
}


static void vpp_opencv_raw(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);

  cv::Mat cv_A = to_opencv(A);

  const int th = state.range_y();
  
  std::vector<cv::KeyPoint> keypoints;
  while (state.KeepRunning())
  {
    keypoints.clear();
    FAST(cv_A, keypoints, th, false);
  }
  //std::cout << "opencv raw " << keypoints.size() << std::endl;
}

static void CustomArguments(benchmark::internal::Benchmark* b) {
  for (int th = 0; th <= 100; th += 2)
    for (int w = 500; w <= 500; w += 300)
      b->ArgPair(w, th);
  //b->ArgPair(500, 1);
}

BENCHMARK(vpp_raw)->Apply(CustomArguments);
//BENCHMARK(vpp_raw_lm)->Apply(CustomArguments);
// BENCHMARK(vpp_raw_bm)->Range(300, 2000);
//BENCHMARK(vpp_opencv_lm)->Apply(CustomArguments);
BENCHMARK(vpp_opencv_raw)->Apply(CustomArguments);
BENCHMARK_MAIN();

// int main(int argc, char* argv[])
// {
//   using namespace vpp;
//   ::sched_param sc_params;
//   sc_params.sched_priority = 10;
//   if (::sched_setscheduler(::getpid(), SCHED_FIFO, &sc_params))
//     ::fprintf(stderr, "sched_setscheduler(): %s\n", ::strerror(errno));

//   if (argc != 4)
//   {
//     std::cerr << "Usage : " << argv[0] << " image threshold local_maxima_only" << std::endl;
//     return 1;
//   }

//   int K = 1000;

//   typedef image2d<vuchar3> I;
//   I A = clone(from_opencv<vuchar3>(cv::imread(argv[1])), _border = 3);
//   I B(A.domain(), _border = 1);

//   image2d<unsigned char> big_image(10000,10000);

//   image2d<unsigned char> Agl(A.domain(), _border = 3);
//   image2d<unsigned char> Bgl(A.domain());
//   pixel_wise(Agl, A) | [] (unsigned char& gl, vuchar3& c)
//   {
//     gl = (c[0] + c[1] + c[2]) / 3;
//   };

//   int lm = atoi(argv[3]);
//   int th = atoi(argv[2]);
//   auto time = get_time_in_seconds();
//   std::vector<vint2> keypoints_vpp;
//   for (unsigned i = 0; i < K; i++)
//   {
//     keypoints_vpp.clear();
//     switch(lm)
//     {
//     case 0: keypoints_vpp = fast_detector9(Agl, th); break;
//     case 1: keypoints_vpp = fast_detector9_local_maxima(Agl, th); break;
//     case 2: keypoints_vpp = fast_detector9_blockwise_maxima(Agl, th, 10); break;
//     }

//     //make_keypoint_vector(tmp, keypoints_vpp);
//   }
//   double vpp_ms_per_iter = 1000 * (get_time_in_seconds() - time) / K;

//   time = get_time_in_seconds();
//   cv::Mat cv_Agl = to_opencv(Agl);
//   cv::Mat cv_B = to_opencv(B);
//   std::vector<cv::KeyPoint> keypoints;
//   double vpp_time = 0, opencv_time = 0;
//   for (unsigned i = 0; i < K; i++)
//   {
//     keypoints.clear();
//     FAST(cv_Agl, keypoints, th, lm);
//   }
//   double opencv_ms_per_iter = 1000 * (get_time_in_seconds() - time) / K;

//   std::cout << "vpp: " << keypoints_vpp.size() << " keypoints" << std::endl;
//   std::cout << "openmp: " << keypoints.size() << " keypoints" << std::endl;

//   std::cout << "Time per iterations: " << std::endl
//             << "OpenCV: " << opencv_ms_per_iter << "ms" << std::endl
//             << "VPP: " << vpp_ms_per_iter << "ms" << std::endl;

//   image2d<vuchar3> vpp_out = clone(A);
//   image2d<vuchar3> opencv_out = clone(A);
//   // pixel_wise(vpp_out, tmp, A) | [] (vuchar3& o, int& k, vuchar3& in)
//   // {
//   //   o = k ? vuchar3(0,0, 255) : in;
//   // };

//   for (int i = 0; i < keypoints.size(); i++)
//     opencv_out(keypoints[i].pt.y, keypoints[i].pt.x) = vuchar3(0, 0, 255);

//   for (int i = 0; i < keypoints_vpp.size(); i++)
//     vpp_out(keypoints_vpp[i]) = vuchar3(0, 0, 255);

//   cv::imwrite("opencv.ppm", to_opencv(opencv_out));
//   cv::imwrite("vpp.ppm", to_opencv(vpp_out));

// }
