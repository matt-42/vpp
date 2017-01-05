
namespace vpp
{

  template <typename SI>
  struct grid_index
  {

    template <typename O>
    grid_index(O&& o)
    {
      search_radius = o.search_radius;
      grid_width = o.grid_width;
      // Fixme build index
    }
    
    template <typename O>
    void index(O&& o)
    {
      idx(o.position / s).index(o);
    }

    template <typename D, typename F>
    void search(O&& o, F distance)
    {
      vint2 prediction = o.position;
      vint2 descriptor = o.descriptor;

      vint2 begin = (prediction - vint2(search_radius, search_radius)) / s;
      vint2 end = (prediction + vint2(search_radius, search_radius)) / s;
      for (vint2 n : box2d(begin, end))
      {
        if (idx.has(n) and idx(n).size() > 0)
        {
          auto& b = idx(n);
          b.search(descriptor, prediction, distance);
        }
      }
    }

    const int S;
    vpp::image2d<SI> idx;
    int s; // scale
    int search_radius;
  };

}
