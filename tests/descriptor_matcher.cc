#include <vector>
#include <vpp/algorithms/descriptor_matcher.hh>
//#include <vpp/algorithms/optical_flow.hh>
#include <vpp/vpp.hh>

template <typename I>
decltype(auto) image_as_array_of_rows(I& img)
{
  return iod::array_view(img.nrows(), [&] (int i) {
      return iod::array_view(img.ncols(), img[i]); });  
}

int main()
{
  using namespace vpp;

  vpp::image2d<int> D1(100, 10);
  vpp::image2d<int> D2(200, 10);

  pixel_wise(D1, D2) | [] (int& a, int& b) { a = rand() % 10000; b = rand() % 10000; };

  struct match { int j; int d; };
  std::vector<match> matches(D1.nrows(), match{-1, -1});

  // Bruteforce
  bruteforce_matcher(D1.nrows(), D2.nrows(),
                     _distance = [&] (int i, int j, int d) {
                       return sad_distance<int>(D1.ncols(), D1[i], D2[j], d); },
                     _match = [&] (int i, int j, int d) { matches[i] = match{j, d}; });

  for (int i = 0; i < D1.nrows(); i++)
  {
    int d = sad_distance<int>(D1.ncols(), D1[i], D2[matches[i].j]);
    assert(d == matches[i].d);

    for (int j = 0; j < D2.nrows(); j++)
      assert(sad_distance<int>(D1.ncols(), D1[i], D2[j]) >= d);
  }


  // Local index1d.
  matches = std::vector<match>(D1.nrows(), match{-1, -1});

  std::vector<vint2> P1, P2; // keypoint positions.
  for (int i = 0; i < D1.nrows(); i++)
    P1.push_back(vint2(rand() % 1000, rand() % 1000));
  for (int i = 0; i < D2.nrows(); i++)
    P2.push_back(vint2(rand() % 1000, rand() % 1000));
  
  local_index1d_sad_descriptor_matcher
    (_query_positions = P1,
     _train_positions = P2,
     _query = image_as_array_of_rows(D1),
     _train = image_as_array_of_rows(D2),
     _approximation = 1,
     _search_radius = 10000,
     _cell_width = 300,
     _distance = [&] (int i, int j, int d) {
      assert(i < D1.nrows());
      assert(j < D2.nrows());
      return sad_distance<int>(D1.ncols(), D1[i], D2[j], d);
     },
     _match = [&] (int i, int j, int d) { matches[i] = match{j, d}; });

  for (int i = 0; i < D1.nrows(); i++)
  {
    assert(matches[i].j < D2.nrows());
    assert(matches[i].j >= 0);
    int d = sad_distance<int>(D1.ncols(), D1[i], D2[matches[i].j]);
    assert(d == matches[i].d);

    for (int j = 0; j < D2.nrows(); j++)
      assert(sad_distance<int>(D1.ncols(), D1[i], D2[j]) >= d);
  }
  
}
