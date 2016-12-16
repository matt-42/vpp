#pragma once

#include <vpp/core/image2d.hh>
#include <vpp/draw/symbols.hh>


namespace vpp
{

  using s::_width;
  using s::_stroke;
  using s::_fill;
  using s::_center;
  
  namespace draw
  {
    template <typename V, typename... OPTS>
    void square(image2d<V> img, OPTS... opts)
    {
      auto options = D(opts...);

      vint2 center = options.center;
      int width = options.width;

      if (options.has(_fill))
      {
        vint2 hdiag(width / 2, width / 2);
        box2d bb(center - hdiag, center + hdiag);

        pixel_wise(img | bb) | [&] (auto& p) {
          p = options.get(_fill, V());
        };

      }
      else
        if (options.has(_stroke))
        {
          // FIXME.
        }
    }
  }
}

