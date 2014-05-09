#include <iostream>
#include <vpp/boxNd.hh>
#include <vpp/vpp.hh>

#include "get_time.hh"

using namespace vpp;

template <typename I, typename F>
void fast_pixel_wise(I img, F f)
{
  int nr = img.nrows();
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* cur = &img(vint2(r, 0));
    // vint2 cur_(r,0);
    int* end = cur + img.nrows();
    int c = 0;
    while (cur != end)
    {
      f(cur, r, c);
      //f(cur, cur_[0], cur_[1]);
      ++cur;
      // cur_[1]++;
      c++;
    }
  }

}

int main()
{

  image2d<int> img(2000,2000);

  int K = 400;
  double time;


  // check
  vpp::pixel_wise(img) << [] (auto&& p)
  {
    p = 42;
  };

  for (auto p : img.domain())
  {
    if (img(p) != 42)
      std::cout << "error at " << p.transpose() << " -> " << img(p) << std::endl;
  }

  // Cache warm up.
  for (int k = 0; k < K; k++)
    fast_pixel_wise(img, [] (int* p, int r, int c)
    {
      *p = r + c;
    });

  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
  {
    int nr = img.nrows();
#pragma omp parallel for
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
  double ref_time = get_time_in_seconds() - time;


  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    fast_pixel_wise(img, [] (int* p, int r, int c)
    {
      *p = r + c;
    });
  double ref2_time = get_time_in_seconds() - time;

  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    for (auto& p : img)
      p = p.coord()[0] + p.coord()[1];
  double ii_time = get_time_in_seconds() - time;

  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    vpp::pixel_wise(img, img.domain()) << [] (auto& p, auto& p2)
    {
      p = p2[0] + p2[1];
    };
  double pw_time = get_time_in_seconds() - time;

  std::cout << "image iterator: " << std::endl;
  std::cout << "ref_time (ms): " << 1000. * ref_time / K << std::endl;
  std::cout << "ref2_time (ms): " << 1000. * ref2_time / K << std::endl;
  std::cout << "ii_time (ms): " << 1000. * ii_time / K << std::endl;
  std::cout << "image iterator overhead: " << 100. * ii_time / ref_time - 100. << "%" << std::endl;
  std::cout << "pw_time (ms): " << 1000. * pw_time / K << std::endl;
  //std::cout << "domain iteration overhead: " << 100. * id_time / ref_time - 100. << "%" << std::endl;

}
