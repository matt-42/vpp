#include <fstream>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>

#include <vpp/algorithms/distance_transforms/distance_transforms.hh>

#include <iod/symbols.hh>
#include <vpp/vpp.hh>

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

using namespace vpp;

const vint2 north(-1, 0);
const vint2 south(1, 0);
const vint2 east(0, 1);
const vint2 west(0, -1);

template <typename T, typename U>
void manual_d4_distance_transform(vpp::image2d<T>& input, vpp::image2d<U>& sedt)
{
  fill_with_border(sedt, INT_MAX / 2);
  pixel_wise(input, sedt) | [] (auto& i, auto& s) { if (i == 0) s = 0; };

  pixel_wise(relative_access(sedt))(_top_to_bottom, _left_to_right, _no_threads) | [&] (auto sn)
  {
    U min_dist = std::min(sn(0,0), U(sn(north) + 1));
    sn(0,0) = std::min(min_dist, U(sn(west) + 1));
  };

  pixel_wise(relative_access(sedt))(_bottom_to_top, _right_to_left, _no_threads) | [&] (auto sn) {
    U min_dist = std::min(sn(0,0), U(sn(south) + 1));
    sn(0,0) = std::min(min_dist, U(sn(east) + 1));
  };
}


template <typename T, typename U>
void manual_d4_distance_transform2(vpp::image2d<T>& input, vpp::image2d<U>& sedt)
{
  using namespace vpp;

  T* in = &input(0,0);
  U* out = &sedt(0,0);

  int nr = input.nrows();
  int nc = input.ncols();

  std::size_t in_pitch = input.pitch();
  std::size_t out_pitch = sedt.pitch();

  fill_with_border(sedt, INT_MAX / 2);

  // Init.
  for (int r = 0; r < nr; r++)
  {
    T* r_in = (T*)(((char*)in) + in_pitch * r);
    U* r_out = (U*)(((char*)out) + out_pitch * r);

    for (int c = 0; c < nc; c++)
      r_out[c] = r_in[c] == 0 ? 0 : INT_MAX / 2;
  }

  // Forward.
  for (int r = 0; r < nr; r++)
  {
    U* rp_out = (U*)(((char*)out) + out_pitch * (r - 1));
    U* r_out = (U*)(((char*)out) + out_pitch * r);

    for (int c = 0; c < nc; c++)
    {
      U min_dist = std::min(r_out[c], U(rp_out[c] + 1));
      r_out[c] = std::min(min_dist, U(r_out[c - 1] + 1));
    }
  }

  // Backward.
  for (int r = nr -1; r >= 0; r--)
  {
    U* rn_out = (U*)(((char*)out) + out_pitch * (r + 1));
    U* r_out = (U*)(((char*)out) + out_pitch * r);

    for (int c = nc - 1; c >= 0; c--)
    {
      U min_dist = std::min(r_out[c], U(rn_out[c] + 1));
      r_out[c] = std::min(min_dist, U(r_out[c + 1] + 1));
    }
  }
}

int main(int argc, char* argv[])
{
  using namespace std;


  {
    using namespace vpp;
    image2d<vuchar3> A = from_opencv<vuchar3>(cv::imread(argv[1]));
    image2d<int> B = pixel_wise(A) | [] (auto& a) -> int {
      return a.norm() > 0 ? 255 : 0;
    };

    // image2d<int> B(300, 300);
    // fill(B, 255);
    // B(150, 150) = 0;
    image2d<int> C4(B.domain(), _border = 2);
    image2d<int> C8(B.domain(), _border = 2);
    image2d<int> CE(B.domain(), _border = 2);
    image2d<int> C34(B.domain(), _border = 2);
    image2d<int> C711(B.domain(), _border = 4);
    manual_d4_distance_transform2(B, C4);
    d8_distance_transform(B, C8);
    euclide_distance_transform(B, CE);
    d3_4_distance_transform(B, C34);
    d5_7_11_distance_transform(B, C711);
    typedef unsigned char US;
    image2d<unsigned char> D4 = pixel_wise(C4) | [] (auto& c) -> US { return c > 255 ? 255 : c; };
    image2d<unsigned char> D8 = pixel_wise(C8) | [] (auto& c) -> US { return c > 255 ? 255 : c; };
    image2d<unsigned char> DE = pixel_wise(CE) | [] (auto& c) -> US { return sqrt(c) > 255 ? 255 : sqrt(c); };
    image2d<unsigned char> D34 = pixel_wise(C34) | [] (auto& c) -> US { return c > 255 ? 255 : c; };
    image2d<unsigned char> D711 = pixel_wise(C711) | [] (auto& c) -> US { return c > 255 ? 255 : c; };

    cv::imwrite("d4.pgm", to_opencv(D4));
    cv::imwrite("d8.pgm", to_opencv(D8));
    cv::imwrite("de.pgm", to_opencv(DE));
    cv::imwrite("d34.pgm", to_opencv(D34));
    cv::imwrite("d711.pgm", to_opencv(D711));
  }

  std::vector<std::vector<pair<int, float>>> results;

  auto add = [] (auto a, auto b) {
    vpp::pixel_wise(a, b) | [] (auto& aa, auto& bb)
    {
      bb = aa + bb;
    };
  };

  auto image_size_bench = [&] (auto f)
  {
    std::vector<pair<int, float>> res;
    for (int s = 10; s < 2000; s += 100)
    {
      using namespace vpp;
    
      image2d<int> img(s, s, _border = 2);
      image2d<int> out(img.domain(), _border = 2);
    
      res.push_back(make_pair(s * s,
                                  benchmark(f, img, out)));
    }
    results.push_back(res);
  };

  image_size_bench(add);
  image_size_bench(manual_d4_distance_transform2<int, int>);
  image_size_bench(vpp::d4_distance_transform);
  image_size_bench(vpp::d8_distance_transform);
  image_size_bench(vpp::d3_4_distance_transform);
  image_size_bench(vpp::euclide_distance_transform<int, int>);
  image_size_bench(vpp::d5_7_11_distance_transform);

  std::vector<string> files = {"bench_add.txt", "manual_d4.txt", "d4.txt",
                          "d8.txt", "d34.txt", "euclide.txt", "d5_7_11.txt"};

  int i = 0;
  for (auto fn : files)
  {
    ofstream f(fn);
    for (auto p : results[i]) f << p.first <<  "\t" << p.second << endl;
    i++;
  }
}
