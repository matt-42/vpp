#include <fstream>
#include <vpp/vpp.hh>

#include <vpp/algorithms/distance_transforms/distance_transforms.hh>

#include <iod/symbols.hh>
#include <vpp/vpp.hh>
#include <vpp/core/liie.hh>

#include "get_time.hh"

template <typename F, typename... A>
float benchmark(F f, A&&... args)
{
  double time = get_time_in_seconds();
  int K = 10;
  for (int k = 0; k < K; k++)
    f(std::forward<A>(args)...);

  return (get_time_in_seconds() - time) / K;
}

int main(int argc, char* argv[])
{
  using namespace std;
  
  vector<pair<int, float>> results;
  vector<pair<int, float>> results1;
  vector<pair<int, float>> results2;

  auto add = [] (auto a, auto b) {
    vpp::pixel_wise(a, b) | [] (auto& aa, auto& bb)
    {
      bb = aa + bb;
    };
  };

  std::cout << "Add: " << std::endl;
  for (int s = 10; s < 2000; s += 100)
  {
    using namespace vpp;
    
    image2d<int> img(s, s, _Border = 1);
    image2d<int> out(img.domain(), _Border = 1);
    
    results.push_back(make_pair(s * s,
                                benchmark(add, img, out)));
    std::cout << results.back().first << " " << results.back().second << endl;
  }

  std::cout << "D4: " << std::endl;
  for (int s = 10; s < 2000; s += 100)
  {
    using namespace vpp;
    image2d<int> img(s, s, _Border = 1);
    image2d<int> out(img.domain(), _Border = 1);
    
    results.push_back(make_pair(s * s,
                                benchmark([] (auto& a, auto& b) { d4_distance_transform(a, b); },
                                          img, out)));
    std::cout << results.back().first << " " << results.back().second << endl;
  }

  std::cout << "D8: " << std::endl;
  for (int s = 10; s < 2000; s += 100)
  {
    using namespace vpp;
    image2d<int> img(s, s, _Border = 1);
    image2d<int> out(img.domain(), _Border = 1);
    
    results.push_back(make_pair(s * s,
                                benchmark([] (auto& a, auto& b) { d8_distance_transform(a, b); },
                                          img, out)));
    std::cout << results.back().first << " " << results.back().second << endl;
  }
  
  std::cout << "Euclide: " << std::endl;
  
  for (int s = 10; s < 2000; s += 100)
  {
    using namespace vpp;
    
    image2d<int> img(s, s, _Border = 1);
    image2d<int> out(img.domain(), _Border = 1);
    
    results.push_back(make_pair(s * s,
                                benchmark([] (auto& a, auto& b) { euclide_distance_transform(a, b); },
                                          img, out)));
    std::cout << results.back().first << " " << results.back().second << endl;
  }
  
  ofstream f(argv[1]);
  for (auto p : results) f << p.first <<  "\t" << p.second << endl;
}
