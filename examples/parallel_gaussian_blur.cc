#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <vpp/vpp.hh>
#include <vpp/opencv_bridge.hh>


inline double get_time_in_seconds()
{
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return double(ts.tv_sec) + double(ts.tv_nsec) / 1000000000.;
}

int main(int argc, char* argv[])
{
  using namespace vpp;

  if (argc != 3)
  {
    std::cerr << "Usage : " << argv[0] << " image filter_width" << std::endl;
    return 1;
  }

  double time;

  int w = atoi(argv[2]);
  if (!(w % 2)) w += 1;

  int hw = w / 2;
  cv::Mat m = cv::imread(argv[1]);
  //image2d<vuchar3> img_(1000,1000);
  image2d<vuchar3> img_;
  //img = image2d<vuchar3>(1000,1000);
  //img = from_opencv<vuchar3>(m);
  img_ = from_opencv<vuchar3>(cv::imread(argv[1]));
  image2d<vuchar4> img(img_.domain());
  //image2d<vuchar3> img = from_opencv<vuchar3>(m);
  image2d<vuchar3> out_(img.domain());
  image2d<vuchar4> out(img.domain());


  for (auto p : img.domain()) img(p).segment(0,3) = img_(p);

  //Cache warm up.
  for (int k = 0; k < 10; k++)
    for (auto p : img.domain())
    {
      vint4 res = vint4::Zero();
      for(auto n : box_nbh(p, w)) if (img.has(n)) res += img(n).cast<int>();
      out(p) = (res / (w * w)).cast<unsigned char>();
    };

  std::cout << img.nrows() << std::endl;
  std::cout << img.ncols() << std::endl;

  std::cout << img.data() << std::endl;
  std::cout << out.data() << std::endl;

  time = get_time_in_seconds();

  // Openmp VPP parallel version.
  parallel_for_openmp(img.domain()) << [&] (vint2 p)
  {
    vint4 res = vint4::Zero();
    for(auto n : box_nbh(p, w)) if (img.has(n)) res += img(n).cast<int>();
    out(p) = (res / (w * w)).cast<unsigned char>();
  };
  std::cout << "multi thread vpp: "<< 1000*(get_time_in_seconds() - time) << "ms" << std::endl;

  // neighborhood nbh(img);

  // parallel_for_openmp(img, out) << [&] (pixel in, pixel out)
  // {
  //   vint4 res = vint4::Zero();
  //   for(auto n : nbh(in, nbh)) res += n->cast<int>();
  //   out = (res / (w * w)).cast<unsigned char>();
  // };

  // [&] (pixel in)
  // {
  //   // Shift registers.
  //   a<-1,-1>() = a<-1,0>();
  //   a< 0,-1>() = a< 0,0>();
  //   a< 1,-1>() = a< 1,0>();

  //   a<-1, 0>() = a<-1,1>();
  //   a< 0, 0>() = a< 0,1>();
  //   a< 1, 0>() = a< 1,1>();

  //   // Load right registers:
  //   a<-1, 1>() = load((char*)in.addr() - pitch);
  //   a< 0, 0>() = load(in.addr());
  //   a< 1, 1>() = load((char*)in.addr() + pitch);
  // };

  // parallel_for(box1d(0, img.ncols())) << [&] (int r)
  // {
  //   simd_tile2d tile;
  //   vuchar4 v;
  //   simd_iterator it;
  //   simd_iterator end;
  //   simd_iterator out_it;

  //   while (it != end)
  //   {
  //     out_it = zero();

  //     out_it += tile(-1, 0);
  //     out_it += tile(0, 0);
  //     out_it += tile(1, 0);

  //     out_it += tile(-1, -1);
  //     out_it += tile(0, -1);
  //     out_it += tile(1, -1);

  //     out_it += tile(-1, 1);
  //     out_it += tile(0, 1);
  //     out_it += tile(1, 1);

  //     tile.shift();
  //     out_it++;
  //   }
  // }

  // parallel_for_openmp(img, out) << [&] (pixel in, pixel out)
  // {
  //   a.next(in);
  //   V res = V::Zero();
  //   for(auto n : nbh(in, nbh)) res += n->cast<int>();
  //   out = (res / (w * w)).cast<unsigned char>();
  // };

  time = get_time_in_seconds();

  for (unsigned i = 0; i < 10; i++)
  parallel_for_pixel_openmp(img, out) << [&] (pixel<vuchar4, vint2>& in,
                                              pixel<vuchar4, vint2>& out_it)
  {
    vint4 res = vint4::Zero();
    for (int r = -hw; r <= hw; r++)
    for (int c = -hw; c <= hw; c++)
    {
      vuchar4* n = ((vuchar4*)((((char*)(in.addr())) + r * img.pitch() + c * sizeof(vuchar4))));
      if (img.has(n))
        res += n->cast<int>();
    }

    out_it = (res / (w * w)).cast<unsigned char>();
  };

  for (auto p : img.domain()) out_(p) = out(p).segment(0,3);
  cv::imwrite("out.jpg", to_opencv(out_));

  std::cout << "multi thread pixel vpp: "<< 1000*(get_time_in_seconds() - time) << "ms" << std::endl;

  time = get_time_in_seconds();

  // Single thread version.
  for (unsigned i = 0; i < 10; i++)
  for (auto p : img.domain())
  {
    vint4 res = vint4::Zero();
    for(auto n : box_nbh(p, w)) if (img.has(n)) res += img(n).cast<int>();
    out(p) = (res / (w * w)).cast<unsigned char>();
  };

  std::cout << "single thread vpp: "<< 1000*(get_time_in_seconds() - time) << "ms" << std::endl;

  // cv::imwrite("in_before.jpg", to_opencv(img));

  // Openmp only version without boundchecking.
  time = get_time_in_seconds();
  for (unsigned K = 0; K < 10; K++)
#pragma omp parallel for
  for (int r = hw; r < img.nrows() - hw; r++)
  {
    vuchar4* row = img.address_of(vint2(r, hw));
    vuchar4* row_end = img.address_of(vint2(r, img.ncols() - hw));
    vuchar4* out_row = out.address_of(vint2(r, hw));
    int pitch = img.pitch();
    while (row != row_end)
    {
      vint4 res = vint4::Zero();

      // char* row2 = (char*)row + (-hw) * pitch;
      // for (int nr = -hw; nr <= hw; nr++)
      //   for (int nc = -hw; nc <= hw; nc++)
      //     res += ((vuchar4*)((char*)(row) + nr * pitch + nc * sizeof(vuchar4)))->cast<int>();

      char* row2 = (char*)row + (-hw) * pitch;
      for (int nr = -hw; nr <= hw; nr++)
      {
        char* cur = row2 - hw * sizeof(vuchar4);
        char* row2_end = row2 + (hw + 1) * sizeof(vuchar4);
        while (cur != row2_end)
        {
          res += ((vuchar4*)(cur))->cast<int>();
          cur += sizeof(vuchar4);
        }
        row2 += pitch;
        for (int nc = -hw; nc <= hw; nc++)
          res += (row + nr * pitch + nc)->cast<int>();
      }
      *out_row = (res / (w * w)).cast<unsigned char>();

      row++;
      out_row++;
    }
  }

  std::cout << "raw multi thread: " << 1000*(get_time_in_seconds() - time) << "ms" << std::endl;
  cv::imwrite("in.jpg", to_opencv(img));
  for (auto p : img.domain()) out_(p) = out(p).segment(0,3);
  cv::imwrite("out.jpg", to_opencv(out_));
}
