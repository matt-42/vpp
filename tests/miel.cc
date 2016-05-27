#include <vpp/vpp.hh>
#include <vpp/algorithms/miel/miel.hh>

int main()
{
  using namespace vpp;
  image2d<uint8_t> img(10, 10, _border = 3);

  fill_with_border(img, 0);

  for (auto p : img.domain())
    assert(img(p) == img[p[0]][p[1]]);
  
  // auto v = miel_detector_blockwise(img, 10, 5);
  // assert(v.size() == 0);

  img(5, 5) = 100;

  auto v2 = miel_detector_blockwise(img, 10, 10);
  //assert(v2.size() == 1);
  
  std::cout << v2.size() << std::endl;
  if (v2.size() > 0)
    std::cout << v2[0].transpose() << std::endl;
  return 0;
}
