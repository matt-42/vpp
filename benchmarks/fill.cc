#include <benchmark/benchmark.h>
#include <vpp/vpp.hh>

using namespace vpp;

static void BM_fill(benchmark::State& state) {
  image2d<int> A(200,200);
  while (state.KeepRunning())
  {
    fill(A, 0);
  }
}

BENCHMARK(BM_fill);

static void BM_memset(benchmark::State& state) {
  auto arr = new int[200 * 200];
  while (state.KeepRunning())
  {
    memset(arr, 0, sizeof(int) * 200 * 200);
  }
  delete arr;
}

// Register the function as a benchmark
BENCHMARK(BM_memset);


static void BM_fill_with_border(benchmark::State& state) {
  image2d<int> A(190,190, _border = 5);
  while (state.KeepRunning())
  {
    fill_with_border(A, 0);
  }
}
BENCHMARK(BM_fill_with_border);

static void BM_fill_border_with_value(benchmark::State& state) {
  image2d<int> A(190,190, _border = 5);
  while (state.KeepRunning())
  {
    fill_border_with_value(A, 0);
  }
}
BENCHMARK(BM_fill_border_with_value);


static void BM_fill_border_closest(benchmark::State& state) {
  image2d<int> A(190,190, _border = 5);
  while (state.KeepRunning())
  {
    fill_border_closest(A);
  }
}
BENCHMARK(BM_fill_border_closest);


static void BM_fill_border_mirror(benchmark::State& state) {
  image2d<int> A(190,190, _border = 5);
  while (state.KeepRunning())
  {
    fill_border_mirror(A);
  }
}
BENCHMARK(BM_fill_border_mirror);

BENCHMARK_MAIN();
