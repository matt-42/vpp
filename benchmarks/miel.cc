#include <benchmark/benchmark.h>

#include <vpp/vpp.hh>
#include <vpp/algorithms/fast_detector/fast.hh>
#include <vpp/algorithms/miel/miel.hh>

using namespace vpp;
static void fast(benchmark::State& state) {

  image2d<unsigned char> img(500, 500, _border = 3);

  fill_with_border(img, 0);
  
  while (state.KeepRunning())
    fast_detector9_blockwise_maxima(img, 10, 10);
}
BENCHMARK(fast);


static void miel(benchmark::State& state) {

  image2d<unsigned char> img(500, 500, _border = 3);

  fill_with_border(img, 0);
  
  while (state.KeepRunning())
    miel_detector_blockwise(img, 10, 10);
}

BENCHMARK(miel);


BENCHMARK_MAIN();
