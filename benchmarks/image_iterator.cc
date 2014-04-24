#include <iostream>
#include <vpp/boxNd.hh>
#include <vpp/image2d.hh>

#include "get_time.hh"

int main()
{
  using namespace vpp;

  image2d<int> img(1000,1000);

  // Cache warm up.
  for (auto p : img)
    p = 42;

  int K = 400;
  double time;

  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
  {
    int* cur = &img(vint2(0, 0));
    for (int r = 0; r < img.nrows(); r++)
    {
      int* end = cur + img.nrows();
      int c = 0;
      while (cur != end)
      {
        *(cur++) = r + c;
        c++;
      }

    }
  }
  double ref_time = get_time_in_seconds() - time;

  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    for (auto p : img)
      p = p.coord()[0] + p.coord()[1];
  double ii_time = get_time_in_seconds() - time;


  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    for (auto p : img.domain())
      img(p) = p[0] + p[1];
  double id_time = get_time_in_seconds() - time;

  std::cout << "image iterator: " << std::endl;
  std::cout << "ref_time (ms): " << 1000. * ref_time / K << std::endl;
  std::cout << "io_time (ms): " << 1000. * ii_time / K << std::endl;
  std::cout << "image iterator overhead: " << 100. * ii_time / ref_time - 100. << "%" << std::endl;
  std::cout << "id_time (ms): " << 1000. * id_time / K << std::endl;
  std::cout << "domain iteration overhead: " << 100. * id_time / ref_time - 100. << "%" << std::endl;

}
