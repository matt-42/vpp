#pragma once

#include <vpp/core/image2d.hh>
#include <vpp/draw/draw.hh>
#include <vpp/core/keypoint_trajectory.hh>
#include <iod/sio_utils.hh>
#include <vpp/algorithms/line_tracker_4_sfm/miscellanous.hh>
#include <opencv2/highgui.hpp>
#include "draw_line_hough.hh"
#include "track.hh"

using namespace vpp;
using namespace cv;

namespace vpp {


void paint_hough_video(std::vector<track>& trs,
                       image2d<vuchar4>& paint_buffer)
{
    // Decrease opacity.
    pixel_wise(paint_buffer) | [] (vuchar4& v) {
        v[3] *= 0.97;
    };

#pragma omp parallel for
    for (int i = 0; i < trs.size(); i++)
    {
        auto& t = trs[i];
        if (!t.isAlive() or t.size() == 0) continue;
        //if (t.alive() && t.size() > 1)
        {
            vint2 p1 = t.position_at_frame(t.last_frame()).template cast<int>();
            vint2 p2 = t.position_at_frame(t.last_frame() - 1).template cast<int>();
            vint2 p3 = t.position_at_frame(t.last_frame() - std::min(t.size() - 1, 10)).template cast<int>();

            float speed = (p3 - p1).norm();
            vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p3[0] - p1[0], p3[1] - p1[1])) * 180.f / M_PI,
                    1.f, 1.f);

            float alpha = std::min(1.f, speed / 10);

            auto paint = [&] (vint2 x, float a)
            {
                if (!paint_buffer.has(x)) return;
                vuchar4 c;
                c.template segment<3>(0) = pt_color;
                c[3] = 255*1*alpha;
                vpp::draw::plot_color(paint_buffer, x, c);
            };

            auto paint2 = [&] (vint2 x, float a, int d)
            {
                if (!paint_buffer.has(x)) return;
                vuchar4 c;
                c.template segment<3>(0) = pt_color;
                c[3] = 255*0.7*alpha;
                vpp::draw::plot_color(paint_buffer, x, c);
            };

            if ((p1 - p3).norm() > 5)
                draw::line2d(p1, p2, paint, paint2);
        }
    }
}

void paint_original_video(std::vector<track>& trs,
                          image2d<vuchar4>& paint_buffer,int T_theta,
                          int nrows,int ncols)
{
    // Decrease opacity.
    pixel_wise(paint_buffer) | [] (vuchar4& v) {
        v[3] *= 0.97;
    };

#pragma omp parallel for
    for (int i = 0; i < trs.size(); i++)
    {
        auto& t = trs[i];
        if (!t.isAlive() or t.size() == 0) continue;

        //if (t.alive() && t.size() > 1)
        {
            vint2 p1 = t.position_at_frame(t.last_frame()).template cast<int>();
            vint2 p2 = t.position_at_frame(t.last_frame() - 1).template cast<int>();
            vint2 p3 = t.position_at_frame(t.last_frame() - std::min(t.size() - 1, 10)).template cast<int>();

            float speed = (p3 - p1).norm();
            vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p3[0] - p1[0], p3[1] - p1[1])) * 180.f / M_PI,
                    1.f, 1.f);

            float alpha = std::min(1.f, speed / 10);

            auto paint = [&] (vint2 x, float a)
            {
                if (!paint_buffer.has(x)) return;
                vuchar4 c;
                //c.template segment<3>(0) = pt_color;
                c.template segment<3>(0) = t.color;
                c[3] = 255*1*alpha;
                vpp::draw::plot_color(paint_buffer, x, c);
            };

            auto paint2 = [&] (vint2 x, float a, int d)
            {
                if (!paint_buffer.has(x)) return;
                vuchar4 c;
                c.template segment<3>(0) = t.color;
                c[3] = 255*0.7*alpha;
                vpp::draw::plot_color(paint_buffer, x, c);
            };

            if ((p1 - p3).norm() > 5)
            {
                float cosinus,sinus;
                vint4 ligne = getLineFromPoint_Fast(p1[0],p1[1],T_theta,nrows,ncols,cosinus,sinus);
                draw::line2d(vint2(ligne[1],ligne[0]), vint2(ligne[3],ligne[2]), paint, paint2);
                ligne = getLineFromPoint_Fast(p2[0],p2[1],T_theta,nrows,ncols,cosinus,sinus);
                draw::line2d(vint2(ligne[1],ligne[0]), vint2(ligne[3],ligne[2]), paint, paint2);
                /*float cosinus,sinus;
                image2d<uchar> grad_img = t.gradient_image_at_frame(t.last_frame());
                vint4 ligne = getLineFromPoint_Fast(p1[0],p1[1],T_theta,nrows,ncols,cosinus,sinus);
                line2d_hough(vint2(ligne[1],ligne[0]), vint2(ligne[3],ligne[2]), paint, paint2,grad_img);
                ligne = getLineFromPoint_Fast(p2[0],p2[1],T_theta,nrows,ncols,cosinus,sinus);
                grad_img = t.gradient_image_at_frame(t.last_frame() - 1);
                line2d_hough(vint2(ligne[1],ligne[0]), vint2(ligne[3],ligne[2]), paint, paint2,grad_img);*/
            }
        }
    }
}




}
