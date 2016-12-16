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
  image2d<unsigned char> A(state.range(0), state.range(0), _border = 3);
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

  const int th = state.range(1);
  
  while (state.KeepRunning())
  {
    keypoints.clear();
    keypoints = fast9(A, th);
  }
}


static void vpp_raw_lm(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);
  std::vector<vint2> keypoints;
  const int th = state.range(1);
  while (state.KeepRunning())
  {
    keypoints.clear();
    keypoints = fast9(A, th, _local_maxima);
  }
}


static void vpp_raw_bm(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);
  std::vector<vint2> keypoints;
  const int th = state.range(1);
  while (state.KeepRunning())
  {
    keypoints.clear();
    keypoints = fast9(A, th, _blockwise, _block_size = 10);
  }
}


static void vpp_opencv_lm(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);

  cv::Mat cv_A = to_opencv(A);

  const int th = state.range(1);

  std::vector<cv::KeyPoint> keypoints;
  while (state.KeepRunning())
  {
    keypoints.clear();
    FAST(cv_A, keypoints, th, true);
  }
}


static void vpp_opencv_raw(benchmark::State& state)
{
  image2d<unsigned char> A = init_input(state);

  cv::Mat cv_A = to_opencv(A);

  const int th = state.range(1);
  
  std::vector<cv::KeyPoint> keypoints;
  while (state.KeepRunning())
  {
    keypoints.clear();
    FAST(cv_A, keypoints, th, false);
  }
}

static void CustomArguments(benchmark::internal::Benchmark* b) {
  for (int th = 1; th <= 100; th += 5)
    for (int w = 500; w <= 500; w += 300)
      b->ArgPair(w, th);
}

BENCHMARK(vpp_raw)->Apply(CustomArguments);
BENCHMARK(vpp_raw_lm)->Apply(CustomArguments);
BENCHMARK(vpp_raw_bm)->Range(300, 2000);
BENCHMARK(vpp_opencv_lm)->Apply(CustomArguments);
BENCHMARK(vpp_opencv_raw)->Apply(CustomArguments);
BENCHMARK_MAIN();
