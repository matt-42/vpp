#include <memory>
#include <benchmark/benchmark.h>
#include <vpp/vpp.hh>

constexpr int S = 21*21;
constexpr int N = 100;

void static_patch(benchmark::State& state)
{

  volatile int out = 12;
  while(state.KeepRunning())
  {
    std::array<int[S], N> patches;

      #pragma omp simd
    for (int i = 0; i < N; i++)
      for (int j = 0; j < S; j++)
        patches[i][j] = 42;
    
#pragma omp simd
    for (int i = 0; i < N; i++)
      for (int j = 0; j < S; j++)
        patches[i][j]++;

    out += patches[N-1][S-1];
  }
  // std::cout << out << std::endl;
}

void vector_patch(benchmark::State& state)
{

  while(state.KeepRunning())
  {
    std::vector<std::vector<int>> patches(N, std::vector<int>(S));

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j] = 42;

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j]++;
  }
}


void vector_image2d_patch(benchmark::State& state)
{

  while(state.KeepRunning())
  {
    std::vector<vpp::image2d<int>> patches(N);

    for (int i = 0; i < N; i++)
      patches[i] = vpp::image2d<int>(1, S);

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][0][j] = 42;

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][0][j]++;
  }
}

void malloc_patch(benchmark::State& state)
{

  while(state.KeepRunning())
  {
    std::vector<int*> patches(N);
    //int* patches[N];
    for (int i = 0; i < N; i++)
      patches[i] = new int[S];

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j] = 42;
    
    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j]++;

    for (int i = 0; i < N; i++)
      delete[] patches[i];
    
  }
}

void malloc2d_patch(benchmark::State& state)
{

  while(state.KeepRunning())
  {
    int* patches[N];
    int* buffer = new int[N * S];
    for (int i = 0; i < N; i++)
      patches[i] = buffer + i * S;

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j] = 42;
    
    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j]++;


    delete[] buffer;
  }
}

void shared_patch(benchmark::State& state)
{

  while(state.KeepRunning())
  {
    std::vector<std::shared_ptr<int>> patches(N);
    for (int i = 0; i < N; i++)
      patches[i] = std::shared_ptr<int>(new int[S], [] (int* x) { delete[] x; });

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i].get()[j] = 42;
    
    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i].get()[j]++;

    // for (int i = 0; i < N; i++)
    //   delete patches[i];
    
  }
}


void unique_patch(benchmark::State& state)
{

  while(state.KeepRunning())
  {
    std::vector<std::unique_ptr<int[]>> patches;
    patches.reserve(N);
    for (int i = 0; i < N; i++)
      patches.push_back(std::unique_ptr<int[]>(new int[S]));

    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j] = 42;
    
    for (int i = 0; i < N; i++)
      #pragma omp simd
      for (int j = 0; j < S; j++)
        patches[i][j]++;

    // for (int i = 0; i < N; i++)
    //   delete patches[i];
    
  }
}


void image2d_patch(benchmark::State& state)
{

  while(state.KeepRunning())
  {
    vpp::image2d<int> patches(N, S);

#pragma omp simd
    for (int i = 0; i < N; i++)
      for (int j = 0; j < S; j++)
        patches[i][j] = 42;
    
#pragma omp simd
    for (int i = 0; i < N; i++)
      for (int j = 0; j < S; j++)
        patches[i][j]++;

    // for (int i = 0; i < N; i++)
    //   delete patches[i];
    
  }
}

BENCHMARK(static_patch);
BENCHMARK(vector_patch);
BENCHMARK(vector_image2d_patch);
BENCHMARK(malloc_patch);
BENCHMARK(malloc2d_patch);
BENCHMARK(shared_patch);
BENCHMARK(unique_patch);
BENCHMARK(image2d_patch);
BENCHMARK_MAIN();
