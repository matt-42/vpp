// Include the header-only library.
#include <vpp/vpp.hh>

int main()
{
  // import vpp into the current namespace.
  using vpp;


  typedef vuchar3 V; // value type.
  typedef image2d<vuchar3> I; // image type.

  // Declare a 2d image.
  I img;

  // Load an external image using opencv.
  img = from_opencv<V>(cv::imread("image.jpg"));

  // Allocate a second image with the same definition domain.
  I out(img.domain());

  // Iterate on img
  for (auto pix : img) *pix = *pix * 2;

  // 3x3 box filter on img
  window<I> row_win(img, {[0, -1], [0, 0], [0, 1]});

  pixel_wise(img, out) << [&] (auto in, auto out)
  {
    V sum = V::Zero();
    for (auto n : row_win(in)) sum += *n;
    *out = sum / 3;
  };

  int nr = img.nrows();
  row_wise(img, out) << [&] (auto in, auto out)
  {
    for (int c = 0; c < nr; c++)
    {
      vuchar3 sum = vuchar3::Zero();
      for (auto n : row_win(in)) sum += *n;
      *out = sum / 3;

      out++;
      in++;
    }
  };

  int dims[] = {100, 200};
  imageNd<int, 2> img(dims);

}
