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


template <typename T, typename U>
void manual_d4_distance_transform(vpp::image2d<T>& input, vpp::image2d<U>& sedt)
{
  using namespace vpp;
  pixel_wise(input, sedt).eval(_If(_1 > 0) (_2 = INT_MAX) (_2 = 0));
    
  auto sedt_nbh = box_nbh2d<int, 3, 3>(sedt);
  pixel_wise(sedt_nbh)(_Col_forward, _Row_forward) | [&] (auto sn) {
    U min_dist = std::min(sn(0,0), sn.north() + 1);
    sn(0,0) = std::min(min_dist, sn.west() + 1);
  };

  pixel_wise(sedt_nbh)(_Col_backward, _Row_backward) | [&] (auto sn) {
    U min_dist = std::min(sn(0,0), sn.south() + 1);
    sn(0,0) = std::min(min_dist, sn.east() + 1);
  };
}

int main(int argc, char* argv[])
{
  using namespace std;
  
  vector<pair<int, float>> results;
  vector<pair<int, float>> results1;
  vector<pair<int, float>> results2;

  // constexpr std::array<vpp::cvint2, 1> a{vpp::cvint2{1,2}};

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


  std::cout << "manual D4: " << std::endl;
  for (int s = 10; s < 2000; s += 100)
  {
    using namespace vpp;
    image2d<int> img(s, s, _Border = 1);
    image2d<int> out(img.domain(), _Border = 1);
    
    results.push_back(make_pair(s * s,
                                benchmark([] (auto& a, auto& b) { manual_d4_distance_transform(a, b); },
                                          img, out)));
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
