#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>

namespace kitti
{
  using namespace vpp;
  
  
  inline image2d<vfloat3> load_flow(std::string filename)
  {
    image2d<vushort3> tmp
      = from_opencv<vushort3>(cv::imread(filename, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR));
    if (!tmp.has_data()) return image2d<vfloat3>();

    image2d<vfloat3> out(tmp.domain());

    pixel_wise(out.domain(), tmp) | [&] (vint2 p, vushort3 v)
    {
      out(p) = vfloat3{ (float(v[1]) - (1 << 15)) / 64.f, (float(v[2]) - (1 << 15)) / 64.f, float(v[0]) };
    };
    return out;
  }

  template <typename F>
  void foreach_training_pair(std::string kitti_root, int N, F&& fun)
  {
    std::string images_root = kitti_root + "/training/image_0/";
    std::string ref_root = kitti_root + "/training/flow_noc/";

    for (int i = 0; i < N; i++)
    {
      std::stringstream ss;
      ss << std::setfill('0') << std::setw(6) << i;

      std::string i1_filename = images_root + ss.str() + "_10.png";
      std::string i2_filename = images_root + ss.str() + "_11.png";
      std::string ref_filename = ref_root + ss.str() + "_10.png";

      image2d<vuchar3> i1 = from_opencv<vuchar3>(cv::imread(i1_filename));
      image2d<vuchar3> i2 = from_opencv<vuchar3>(cv::imread(i2_filename));

      if (!i1.has_data())
        throw std::runtime_error(std::string("Cannot read image ") + i1_filename);
      if (!i2.has_data())
        throw std::runtime_error(std::string("Cannot read image ") + i2_filename);

      image2d<vfloat3> ref_flow = load_flow(ref_filename);

      fun(i1, i2, ref_flow);
    }
  }
  
  inline void write_flow(std::string filename, const image2d<vfloat2>& flow, const image2d<char>& has_flow)
  {
    image2d<vushort3> out(flow.domain());

    fill(out, vushort3(0,0,0));
  
    pixel_wise(out.domain(), flow) | [&] (vint2 p, vfloat2 flow)
    {
      if (has_flow(p))
      {        
        // red.
        out(p)[2] = (uint16_t)std::max(std::min(flow[1]*64.0f+32768.0f,65535.0f),0.0f);
        // green
        out(p)[1] = (uint16_t)std::max(std::min(flow[0]*64.0f+32768.0f,65535.0f),0.0f);
        // blue
        out(p)[0] = 1;
      }
    };

    cv::imwrite(filename, to_opencv(out), { CV_IMWRITE_PNG_COMPRESSION, 0});
  }

  inline auto flow_error_stats(const image2d<vfloat3>& flow, const image2d<vfloat3>& ref)
  {
    int n = 0;

    float error_sum = 0.f;

    std::vector<float> errors;

    image2d<unsigned char> errors_map(flow.domain());
    fill(errors_map, 0);

    int cpt = 0;
    pixel_wise(flow.domain())(_no_threads) | [&] (vint2 p)
    {
      if (flow(p)[2] > 0.f) cpt++;
      if (flow(p)[2] > 0.f and ref(p)[2] > 0.f)
      {
        n++;
        float err = (flow(p).segment<2>(0) - ref(p).segment<2>(0)).norm();
        error_sum += err;
        errors.push_back(err);
        errors_map(p) = std::min(err * 20.f, 255.f);
      }
    };

    std::sort(errors.begin(), errors.end());

    int n1 = 0;
    int n3 = 0;
    int n5 = 0;
    int n10 = 0;
    for (int i = 0; i < errors.size(); i++)
    {
      if (errors[i] > 1.f) n1++;
      if (errors[i] > 3.f) n3++;
      if (errors[i] > 5.f) n5++;
      if (errors[i] > 10.f) n10++;
    }

    struct R {
      float n1;
      float n3;
      float n5;
      float n10;
      float avg;
      std::vector<float> errors;
      image2d<unsigned char> errors_map;
      float density;
    };


    return R{
      n1 ? 100 * float(n1) / errors.size() : 0,
        n3 ? 100 * float(n3) / errors.size() : 0,
        n5 ? 100 * float(n5) / errors.size() : 0,
        n10 ? 100 * float(n10) / errors.size() : 0,
        error_sum / (errors.size() ? errors.size() : 1),
        errors, errors_map,
        100.f * float(cpt) / (flow.nrows() * flow.ncols())
        };
  }

}
