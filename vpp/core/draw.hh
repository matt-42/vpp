#ifndef VPP_DRAW_HH_
# define VPP_DRAW_HH_

namespace vpp
{
  namespace draw
  {
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
        if (steep)
          to_plot = vint2{x, y};
        else
          to_plot = vint2{y, x};

        if (out.has(to_plot))
          out(to_plot) = color;

        error = error + deltaerr;
        if (error >= 0.5)
        {
          y = y + ystep;
          error = error - 1.0;
        }
      }
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
  }
}

#endif
