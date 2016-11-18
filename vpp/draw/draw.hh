#ifndef VPP_DRAW_HH_
# define VPP_DRAW_HH_

#include <vpp/core/vector.hh>

namespace vpp
{
  namespace draw
  {
  template <typename I, typename V>
  inline void plot_color(image2d<I>& out, vint2 p, V color)
  {
    out(p) = color;
  }
  template <typename I, typename V>
  inline void plot_color_antialias(image2d<I>& out, vint2 p, V color)
  {
    out(p) = color;
  }

    template <typename I, typename V>
    void c9(I out, vint2 a, V color)
    {
      for (int r = -1; r <= 1; r++)
        for (int c = -1; c <= 1; c++)
        {
          vint2 n = a + vint2{r, c};
          if (out.has(n))
            out(n) = color;
        }
    }


    inline void plot_color(image2d<vuchar3>& out, vint2 p, vuchar4 rgba)
    {
      //out(p) = rgba.segment<3>(0);
      vuchar3 color = ((out(p).cast<int>() * (255 - rgba[3]) + rgba.segment<3>(0).cast<int>() * rgba[3]) / 255).cast<unsigned char>();
      out(p) = color;

//      rgba[3] *= 0.5;
//      for (int r = -1; r <= 1; r++)
//        for (int c = -1; c <= 1; c++)
//          if (r != 0 or c != 0)
//          {
//            vint2 n = p + vint2{r, c};
//            if (out.has(n))
//            {
//              color = ((out(n).cast<int>() * (255 - rgba[3]) + rgba.segment<3>(0).cast<int>() * rgba[3]) / 255).cast<unsigned char>();
//              out(n) = color;
//            }
//          }

    }

    inline void plot_color_antialias(image2d<vuchar3>& out, vint2 p, vuchar4 rgba)
    {
      rgba[3] *= 0.6;
      plot_color(out, p, rgba);
    }

    using namespace vpp;
    template <typename I, typename V>
    void line2d(I out, vint2 a, vint2 b, V color)
    {
      int x0 = a[1]; int y0 = a[0];
      int x1 = b[1]; int y1 = b[0];

      int steep = ::abs(y1 - y0) > ::abs(x1 - x0);

      if (steep)
      {
        std::swap(x0, y0);
        std::swap(x1, y1);
      }

      if (x0 > x1)
      {
        std::swap(x0, x1);
        std::swap(y0, y1);
      }

      int deltax = x1 - x0;
      int deltay = ::abs(y1 - y0);
      float error = 0.f;
      float deltaerr = deltay / float(deltax);
      int ystep;
      int y = y0;
      if (y0 < y1) ystep = 1; else ystep = -1;

      for (int x = x0 + 1; x <= x1; x++)
      {
        vint2 to_plot;
        vint2 n1,n2; // line border.
        if (steep)
        {
          to_plot = vint2{x, y};
          n1 = vint2{x, y-1};
          n2 = vint2{x, y+1};
        }
        else
        {
          to_plot = vint2{y, x};
          n1 = vint2{y-1, x};
          n2 = vint2{y+1, x};
        }

        if (out.has(to_plot))
        {
          plot_color(out, to_plot, color);
          if (out.has(n1)) plot_color_antialias(out, n1, color);
          if (out.has(n2)) plot_color_antialias(out, n2, color);
        }

        error = error + deltaerr;
        if (error >= 0.5)
        {
          y = y + ystep;
          error = error - 1.0;
        }
      }
    }

    template <typename I, typename V>
    void square(I out, vint2 a, int size, V color)
    {
      for (int r = -size/2; r <= size/2; r++)
        for (int c = -size/2; c <= size/2; c++)
        {
          vint2 n = a + vint2{r, c};
          if (out.has(n))
            out(n) = color;
        }
    }
  }
}

#endif
