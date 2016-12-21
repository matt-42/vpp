#include <vector>
#include <vpp/algorithms/descriptor_matcher.hh>
//#include <vpp/algorithms/optical_flow.hh>
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp;

  vpp::image2d<int> D1(100, 10);
  vpp::image2d<int> D2(200, 10);

  pixel_wise(D1, D2) | [] (int& a, int& b) { a = rand() % 10000; b = rand() % 10000; };

  struct match { int j; int d; };
  std::vector<match> matches(D1.nrows(), match{-1, -1});

  
  descriptor_matcher(_size1 = D1.nrows(), _size2 = D2.nrows(),
                     _distance = [&] (int i, int j, int d) {
                       return sad_distance<int>(D1.ncols(), D1[i], D2[j], d); },
                     _match = [&] (int i, int j, int d) { matches[i] = match{j, d}; },
                     _bruteforce
                     );

  for (int i = 0; i < D1.nrows(); i++)
  {
    int d = sad_distance<int>(D1.ncols(), D1[i], D2[matches[i].j]);
    assert(d == matches[i].d);

    for (int j = 0; j < D2.nrows(); j++)
      assert(sad_distance<int>(D1.ncols(), D1[i], D2[j]) >= d);
  }

}
