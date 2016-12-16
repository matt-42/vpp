#ifndef VPP_FILL_HH__
# define VPP_FILL_HH__

# include <vpp/core/boxNd.hh>
# include <vpp/core/vector.hh>
# include <vpp/core/pixel_wise.hh>

namespace vpp
{

  template <typename V, typename U, unsigned N>
  void fill(imageNd<V, N>& img, U&& value)
  {
    pixel_wise(img) | [=] (auto& pix) { pix = value; };
  }

  template <typename V, unsigned N>
  void fill(imageNd<V, N>& img, V value, const boxNd<N>& box)
  {
    pixel_wise(box, img) | [=] (auto, auto& pix) { pix = value; };
  }

  template <typename V, typename U, unsigned N>
  void fill_with_border(imageNd<V, N>& img, U&& value)
  {
    auto box = img.domain_with_border();
    pixel_wise(box, img) | [=] (auto, auto& pix) { pix = value; };
  }

  
  template <typename V, typename U>
  void fill_border_with_value(image2d<V>& img, U&& value)
  {
    int border = img.border();
    int nc = img.ncols();
    int nr = img.nrows();

    auto top = box2d({-border, -border}, {-1, nc + border - 1});
    auto bottom = box2d({nr, -border}, {nr + border - 1, nc + border - 1});
    auto left = box2d({0, -border}, {nr - 1, -1});
    auto right = box2d({0, nc}, {nr - 1, nc + border - 1});

    for (auto b : {top, bottom, left, right})
      pixel_wise(b, img)(_no_threads) | [&] (auto, auto& pix) { pix = value; };
  }

  template <typename V>
  void fill_border_mirror(image2d<V>& img)
  {
    int border = img.border();
    int nc = img.ncols();
    int nr = img.nrows();

    // 1  2  3    
    // 4     5
    // 6  7  8
  
    // Corners.
    pixel_wise(box2d({-border, -border}, {-1, -1}), img)(_no_threads) | // 1
      [&] (auto p, auto& pix) { pix = img(-p[0] - 1, -p[1] - 1); };

    pixel_wise(box2d({-border, nc}, {-1, nc + border - 1}), img)(_no_threads) | // 3
      [&] (auto p, auto& pix) { pix = img(-p[0] - 1, 2 * nc - p[1] - 1); };

    pixel_wise(box2d({nr, -border}, {nr + border -1, -1}), img)(_no_threads) | // 6
      [&] (auto p, auto& pix) { pix = img(2 * nr - p[0] - 1, -p[1] - 1); };
 
    pixel_wise(box2d({nr, nc}, {nr + border - 1, nc + border - 1}), img)(_no_threads) | // 8
      [&] (auto p, auto& pix) { pix = img(2 * nr - p[0] - 1, 2 * nc - p[1] - 1); };

    // Edges.
    pixel_wise(box2d({-border, 0}, { - 1, nc - 1}), img)(_no_threads) | // 2
      [&] (auto p, auto& pix) { pix = img(-p[0] - 1, p[1]); };

    pixel_wise(box2d({nr, 0}, {nr + border - 1, nc - 1}), img)(_no_threads) | // 7
      [&] (auto p, auto& pix) { pix = img(2 * nr -p[0] - 1, p[1]); };
    
    pixel_wise(box2d({0, -border}, {nr - 1, -1}), img)(_no_threads) | // 4
      [&] (auto p, auto& pix) { pix = img(p[0], - p[1] - 1); };

    pixel_wise(box2d({0, nc}, {nr - 1, nc + border - 1}), img)(_no_threads) | // 5
      [&] (auto p, auto& pix) { pix = img(p[0],  2 * nc - p[1] - 1); };
  }

  template <typename V>
  void fill_border_closest(image2d<V>& img)
  {
    int border = img.border();
    int nc = img.ncols();
    int nr = img.nrows();

    // Corners.
    V v = img(0, 0);
    pixel_wise(box2d({-border, -border}, {-1, -1}), img)(_no_threads) |
      [=] (auto p, auto& pix) { pix = v; };

    v = img(0, nc - 1);
    pixel_wise(box2d({-border, nc}, {-1, nc + border - 1}), img)(_no_threads) |
      [=] (auto p, auto& pix) { pix = v; };

    v = img(nr - 1, 0);
    pixel_wise(box2d({nr, -border}, {nr + border -1, -1}), img)(_no_threads) |
      [&] (auto p, auto& pix) { pix = v; };

    v = img(nr - 1, nc - 1);
    pixel_wise(box2d({nr, nc}, {nr + border - 1, nc + border - 1}), img)(_no_threads) |
      [&] (auto p, auto& pix) { pix = v; };

    // Edges.
    pixel_wise(box2d({-border, 0}, { - 1, nc - 1}), img)(_no_threads) |
      [&] (auto p, auto& pix) { pix = img(0, p[1]); };

    pixel_wise(box2d({nr, 0}, {nr + border - 1, nc - 1}), img)(_no_threads) |
      [&] (auto p, auto& pix) { pix = img(nr - 1, p[1]); };
    
    pixel_wise(box2d({0, -border}, {nr - 1, -1}), img)(_no_threads) |
      [&] (auto p, auto& pix) { pix = img(p[0], 0); };

    pixel_wise(box2d({0, nc}, {nr - 1, nc + border - 1}), img)(_no_threads) |
      [&] (auto p, auto& pix) { pix = img(p[0], nc - 1); };
  }

  
};

#endif
