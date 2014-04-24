#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/opencv_bridge.hh>

int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 3)
  {
    std::cerr << "Usage : " << argv[0] << " image filter_width" << std::endl;
    return 1;
  }

  int w = atoi(argv[2]);
  if (!(w % 2)) w += 1;

  image2d<vuchar3> img = from_opencv<vuchar3>(cv::imread(argv[1]));
  image2d<vuchar3> out(img.domain());

  std::cout << img.nrows() << std::endl;
  std::cout << img.ncols() << std::endl;

  std::cout << img.data() << std::endl;
  std::cout << out.data() << std::endl;
  // Openmp VPP parallel version.
  // parallel_for_openmp(img.domain()) << [&] (vint2 p)
  // {
  //   vint3 res = vint3::Zero();
  //   for(auto n : box_nbh(p, w)) if (img.has(n)) res += img(n).cast<int>();
  //   out(p) = (res / (w * w)).cast<unsigned char>();
  // };


  // neighborhood nbh(img);
  
  // parallel_for_openmp(img, out) << [&] (pixel in, pixel out)
  // {
  //   vint3 res = vint3::Zero();
  //   for(auto n : nbh(in, nbh)) res += n->cast<int>();
  //   out = (res / (w * w)).cast<unsigned char>();
  // };

  // Single thread version.
  // for (auto p : img.domain())
  // {
  //   vint3 res = vint3::Zero();
  //   for(auto n : box_nbh(p, w)) if (img.has(n)) res += img(n).cast<int>();
  //   out(p) = (res / (w * w)).cast<unsigned char>();
  // };

  cv::imwrite("in_before.jpg", to_opencv(img));


  // Openmp only version without boundchecking.
  int hw = w / 2;
#pragma omp parallel for
  for (int r = hw; r < img.nrows() - hw; r++)
  {
    vuchar3* row = img.address_of(vint2(r, hw));
    vuchar3* row_end = img.address_of(vint2(r, img.ncols() - hw));
    vuchar3* out_row = out.address_of(vint2(r, hw));
    int pitch = img.pitch();
    while (row != row_end)
    {
      vint3 res = vint3::Zero();

      //char* row2 = (char*)row + (-hw) * pitch;
      for (int nr = -hw; nr <= hw; nr++)
        for (int nc = -hw; nc <= hw; nc++)
          res += ((vuchar3*)((char*)(row) + nr * pitch + nc * sizeof(vuchar3)))->cast<int>();

      // char* row2 = (char*)row + (-hw) * pitch;
      // for (int nr = -hw; nr <= hw; nr++)
      // {
      //   char* cur = row2;
      //   char* row2_end = row2 + nr * pitch + sizeof(vuchar3);
      //   while (cur != row2_end)
      //   {
      //     res += ((vuchar3*)(cur))->cast<int>();
      //     cur += sizeof(vuchar3);
      //   }
      //   row2 += pitch;
      //   // for (int nc = -hw; nc <= hw; nc++)
      //   //   res += (row + nr * pitch + nc)->cast<int>();
      // }
      *out_row = (res / (w * w)).cast<unsigned char>();

      row++;
      out_row++;
    }
  }

  cv::imwrite("in.jpg", to_opencv(img));
  cv::imwrite("out.jpg", to_opencv(out));
}
