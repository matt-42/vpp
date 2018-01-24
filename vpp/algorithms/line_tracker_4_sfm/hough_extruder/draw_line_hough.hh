#pragma once

#include <vpp/core/vector.hh>
#include <vpp/draw/square.hh>
#include <vpp/draw/symbols.hh>
#include <list>

namespace vpp
{
using namespace vpp;
using namespace draw;


template <typename V, typename U>
/**
 * @brief line2d_hough : draw a line in a grascale gradient image
 * @param a
 * @param b
 * @param paint
 * @param paint_border
 * @param grad
 * @param line_width
 */
void line2d_hough(vint2 a, vint2 b, V paint, U paint_border,image2d<unsigned char> grad , int line_width = 5)
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
        vint2 d1,d2; // line border.
        if (steep)
        {
            to_plot = vint2{x, y};
            d1 = vint2{0, -1};
            d2 = vint2{0, +1};
        }
        else
        {
            to_plot = vint2{y, x};
            d1 = vint2{-1, 0};
            d2 = vint2{+1, 0};
        }

        if(grad(to_plot) < 10)
            continue;

        float interp = float(x - x0) / (x1 - x0);
        paint(to_plot, interp);
        for (int bi = 1; bi < line_width / 2; bi++)
        {
            paint_border(to_plot + bi * d1, interp, bi);
            paint_border(to_plot + bi * d2, interp, bi);
        }

        error = error + deltaerr;
        if (error >= 0.5)
        {
            y = y + ystep;
            error = error - 1.0;
        }
    }
}


}
