
#include <vpp/vpp.hh>
#include <vpp/algorithms/lbp/lbp_transform.hh>

#include "get_time.hh"

using namespace vpp;

template <typename V, typename U>
void lbp_transform2(image2d<V>& A, image2d<U>& B)
{
  int nr = A.nrows();
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    V* curA = &A(vint2(r, 0));
    U* curB = &B(vint2(r, 0));
    int nc = A.ncols();

    V* rows[3];
    for (int i = -1; i <= 1; i++)
      rows[i + 1] = (V*)&A(vint2(r + i, 0));

    for (int i = 0; i < nc; i++)
    {
      curB[i] =
        ((rows[0][i - 1] > rows[1][i]) << 0) +
        ((rows[0][i    ] > rows[1][i]) << 1) +
        ((rows[0][i + 1] > rows[1][i]) << 2) +

        ((rows[1][i - 1] > rows[1][i]) << 3) +
        ((rows[1][i + 1] > rows[1][i]) << 4) +

        ((rows[2][i - 1] > rows[1][i]) << 5) +
        ((rows[2][i    ] > rows[1][i]) << 6) +
        ((rows[2][i + 1] > rows[1][i]) << 7);
        
    }
  }

}

int main()
{
  using namespace vpp;
  

  //std::vector<std::pair<int, float>> result = benchmark_cpp(100, 1000, 10, [] (int w) {
  for (int i = 100; i < 4000; i+= 100)
  {

    image2d<unsigned char> V(i, i, _Border = 1);
    image2d<unsigned char> lbp(i, i);

    int K = 400;
    float t = time([&] () {
        for (int k = 0; k < K; k++)
          lbp_transform(V, lbp);
      });
    std::cout << "LBP: " << i << ": " << 1e9 * t / (K * i * i) << "ns / pixels." << std::endl;

    t = time([&] () {
        for (int k = 0; k < K; k++)
          pixel_wise(V, lbp) | [] (auto& a, auto& b) {
            a = a + b;
          };
      });
    std::cout << "ADD: " << i << ": " << 1e9 * t / (K * i * i) << "ns / pixels." << std::endl;

  }
}
