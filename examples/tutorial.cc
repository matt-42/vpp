// Include the header-only library.
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>

int main()
{
  // import vpp into the current namespace.
  using namespace vpp;


  typedef vuchar3 V; // value type.
  typedef image2d<vuchar3> I; // image type.

  // Declare a 2d image.
  I img;

  // Load an external image using opencv.
  img = from_opencv<V>(cv::imread("image.jpg"));

  // Allocate a second image with the same definition domain.
  I out(img.domain());

  // Iterate on img
  pixel_wise(img) | [&] (auto& i) { i += vuchar3(1,1,1); };

  // 3x3 box filter on img
  auto nbh = box_nbh2d<vuchar3, 3, 3>(img);

  pixel_wise(nbh, out) | [] (auto& n, auto& b) {
    vint3 sum = vint3::Zero();

    sum += n(0, -1).cast<int>();
    sum += n(0, 0).cast<int>();
    sum += n(0, 1).cast<int>();

    b = (sum / 3).cast<unsigned char>();
  };

  // Sum the rows.
  std::vector<vint3> sums(img.nrows());
  std::cout << img.nrows() << std::endl;
  
  row_wise(img, img.domain())(_no_threads) | [&] (auto row, auto coord)
  {
    vint3 sum = vint3::Zero();
    pixel_wise(row) | [&] (vuchar3 p) { sum += p.cast<int>(); };
    sums[coord.p1()[0]] = sum;
  };

}
