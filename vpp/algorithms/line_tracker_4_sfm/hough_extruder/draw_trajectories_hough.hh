#pragma once

#include <vpp/core/image2d.hh>
#include <vpp/draw/draw.hh>
#include <vpp/core/keypoint_trajectory.hh>
#include <iod/sio_utils.hh>
#include <vpp/algorithms/line_tracker_4_sfm/miscellanous.hh>
#include <opencv2/highgui.hpp>
#include "track.hh"


namespace vppx
{
using namespace vpp;
using namespace cv;

template <typename... OPTS>
/**
 * @brief draw_trajectories_hough : draw point trajectories in the hough space
 * @param out
 * @param trs
 * @param max_trajectory_len
 * @param T_theta
 * @param nrows
 * @param ncols
 * @param opts
 */
void draw_trajectories_hough(image2d<vuchar3>& out, std::vector<keypoint_trajectory>& trs,
                             int max_trajectory_len,int T_theta,int nrows,int ncols,
                             OPTS... opts)
{
    auto options = iod::D(opts...);

    auto trajectory_color = options.get(_trajectory_color, [] (int i) {
        return vuchar3(255, 255, 0); });

    if (max_trajectory_len == -1)
        max_trajectory_len = INT_MAX;

    std::vector<int> sorted_idx;
    for (auto& t : trs) sorted_idx.push_back(sorted_idx.size());

    std::sort(sorted_idx.begin(), sorted_idx.end(), [&] (auto i, auto j) { return trs[i].start_frame() > trs[j].start_frame(); });

#pragma omp parallel for
    for (int ti = 0; ti < trs.size(); ti++)
    {
        keypoint_trajectory& t = trs[sorted_idx[ti]];
        if (!t.alive() or t.size() == 0) continue;

        vint2 p1 = t.position_at_frame(t.end_frame()).template cast<int>();
        vint2 p2 = t.position_at_frame(t.end_frame() - std::min(10, int(t.size()) - 1)).template cast<int>();

        //vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p2[0] - p1[0], p2[1] - p1[1])) * 180.f / M_PI, 1.f, 1.f);

        int last_frame_id = std::max(t.end_frame() - max_trajectory_len, t.start_frame());
        if ((p1 - t.position_at_frame(last_frame_id).template cast<int>()).norm() < 4)
            continue;

        for (int i = t.end_frame(); i >= std::max(t.end_frame() - max_trajectory_len, t.start_frame()) + 1; i--)
        {
            vint2 p1 = t.position_at_frame(i).template cast<int>();
            vint2 p2 = t.position_at_frame(i - 1).template cast<int>();

            vint4 ligne1 = getLineFromPoint(p1[0],p1[1],T_theta,nrows,ncols);
            vint4 ligne2 = getLineFromPoint(p2[0],p2[1],T_theta,nrows,ncols);

            //vector<cv::Point> points(4);
            //points = { (x1,y1),(x2,y2),(x3,y3),(x4,y4) };

            //std::cout << " de : (" << p1[0] << "," << p1[1] << ")  à  (" << p2[0]  << "," << p2[1] << ")" << std::endl;
            vuchar4 color;
            /*color.segment<3>(0) = pt_color;
            color[3] = 0.4f*(255.f - 255.f * (t.end_frame() - i) / max_trajectory_len);*/
            color = vuchar4(0,255,0,255);

            draw::line2d(out, vint2(ligne1[1],ligne1[0]), vint2(ligne1[3],ligne1[2]),
                    color
                    );
            draw::line2d(out, vint2(ligne2[1],ligne2[0]), vint2(ligne2[3],ligne2[2]),
                    color
                    );
        }

        //draw::c9(out, t.position().template cast<int>(), pt_color);
        draw::c9(out, t.position().template cast<int>(), vuchar3(0,0,255));

    }
}

template <typename... OPTS>
/**
 * @brief draw_trajectories_hough_V1
 * @param out
 * @param trs
 * @param max_trajectory_len
 * @param opts
 */
void draw_trajectories_hough_V1(image2d<vuchar3>& out, std::vector<track>& trs,
                                int max_trajectory_len,
                                OPTS... opts)
{
    auto options = iod::D(opts...);

    auto trajectory_color = options.get(_trajectory_color, [] (int i) {
        return vuchar3(255, 255, 0); });

    if (max_trajectory_len == -1)
        max_trajectory_len = INT_MAX;

    std::vector<int> sorted_idx;
    for (auto& t : trs) sorted_idx.push_back(sorted_idx.size());

    std::sort(sorted_idx.begin(), sorted_idx.end(), [&] (auto i, auto j) { return trs[i].first_frame() > trs[j].first_frame(); });

#pragma omp parallel for
    for (int ti = 0; ti < trs.size(); ti++)
    {
        track& t = trs[sorted_idx[ti]];
        if (!t.isAlive() or t.size() == 0) continue;

        vint2 p1 = t.position_at_frame(t.last_frame());
        vint2 p2 = t.position_at_frame(t.last_frame() - std::min(10, int(t.size()) - 1));

        vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p2[0] - p1[0], p2[1] - p1[1])) * 180.f / M_PI, 1.f, 1.f);

        int last_frame_id = std::max(t.last_frame() - max_trajectory_len, t.first_frame());
        if ((p1 - t.position_at_frame(last_frame_id)).norm() < 4)
            continue;

        for (int i = t.last_frame(); i >= std::max(t.last_frame() - max_trajectory_len, t.first_frame()) + 1; i--)
        {
            vint2 p1 = t.position_at_frame(i);
            vint2 p2 = t.position_at_frame(i - 1);
            vuchar4 color;
            color.segment<3>(0) = t.color;
            color[3] = 255;
            draw::line2d(out, p1, p2,
                         color);
        }
        draw::c9(out, t.last_point, t.color);

    }
}


/**
 * @brief draw_dotted_line
 * @param img
 * @param interval
 * @param p1
 * @param p2
 * @param alpha
 * @param color
 */
void draw_dotted_line(cv::Mat& img,int interval,
                      cv::Point p1,cv::Point p2,float alpha,vuchar3 color)
{
    cv::LineIterator it(img,p1,p2,8);
    for(int i = 0; i < it.count; i++,it++)
    {
        cv::Point curr = it.pos();
        //if ( i%5==0 )
        {
            /*(*it)[0] = 0;
            (*it)[1] = 255;
            (*it)[2] = 0;*/
            float b,g,r;
            b = alpha*img.at<cv::Vec3b>(curr)[0] + (1-alpha)*color[0];
            g = alpha*img.at<cv::Vec3b>(curr)[1] + (1-alpha)*color[1];
            r = alpha*img.at<cv::Vec3b>(curr)[2] + (1-alpha)*color[2];
            circle(img,curr,1,Scalar(uchar(b),uchar(g),uchar(r)),CV_FILLED,8,0);
            /*img.at<cv::Vec3b>(curr)[0]  = uchar(b);
            img.at<cv::Vec3b>(curr)[1]  = uchar(g);
            img.at<cv::Vec3b>(curr)[2]  = uchar(r);*/
        }
    }
}



template <typename... OPTS>
/**
 * @brief draw_trajectories_hough_img_ori_tracks
 * @param out
 * @param trs
 * @param max_trajectory_len
 * @param T_theta
 * @param nrows
 * @param ncols
 * @param opts
 */
void draw_trajectories_hough_img_ori_tracks(cv::Mat & out, std::vector<track>& trs,
                                            int max_trajectory_len,int T_theta,int nrows,int ncols,
                                            OPTS... opts)
{
    auto options = iod::D(opts...);

    /*auto trajectory_color = options.get(_trajectory_color, [] (int i) {
        return vuchar3(255, 255, 0); });*/

    if (max_trajectory_len == -1)
        max_trajectory_len = INT_MAX;

    std::vector<int> sorted_idx;
    for (auto& t : trs) sorted_idx.push_back(sorted_idx.size());

    std::sort(sorted_idx.begin(), sorted_idx.end(), [&] (auto i, auto j) { return trs[i].first_frame() > trs[j].first_frame(); });

    //#pragma omp parallel for
    for (int ti = 0; ti < trs.size(); ti++)
    {
        track& t = trs[sorted_idx[ti]];
        if (!t.isAlive() or t.size() == 0) continue;

        vint2 p1 = t.position_at_frame(t.last_frame());
        vint2 p2 = t.position_at_frame(t.last_frame() - std::min(10, int(t.size()) - 1));

        //vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p2[0] - p1[0], p2[1] - p1[1])) * 180.f / M_PI, 1.f, 1.f);

        int last_frame_id = std::max(t.last_frame() - max_trajectory_len, t.first_frame());
        /*if ((p1 - t.position_at_frame(last_frame_id).template cast<int>()).norm() < 4)
            continue;*/

        /*if(t.size()<15)
            continue;*/

        for (int i = t.last_frame(); i >= std::max(t.last_frame() - max_trajectory_len, t.first_frame()) + 1; i--)
        {
            vint2 p1 = t.position_at_frame(i);
            vint2 p2 = t.position_at_frame(i - 1);
            vint4 l1 = t.position_at_frame_image(i);
            vint4 l2 = t.position_at_frame_image(i - 1);

            float alpha = 1/(i+1);
            int inter = int(20/i);
            vuchar3 color = t.color;
            if(inter<3)
                inter = 3;
            draw_dotted_line(out,inter,cv::Point(l1[0],l1[1]), cv::Point(l1[2],l1[3]),alpha,color);
            draw_dotted_line(out,inter,cv::Point(l2[0],l2[1]), cv::Point(l2[2],l2[3]),alpha,color);

            //cv::line(out, cv::Point(l1[0],l1[1]), cv::Point(l1[2],l1[3]), Scalar(0,255,0,0.4),2);
            //cv::line(out, cv::Point(l2[0],l2[1]), cv::Point(l2[2],l2[3]), Scalar(0,255,0,0.4),2);


        }
        //vint4 ori = t.last_point_xy;
        vint2 lp = t.last_point;
        cout << "point " << lp[0] << " , " << lp[1] << endl;
        vint4 ori = /*t.last_point_xy*/getLineFromPoint(lp[0],lp[1],T_theta,nrows,ncols);
        cv::line(out, cv::Point(ori[0],ori[1]), cv::Point(ori[2],ori[3]), Scalar(0,0,255),3);
    }
}

/**
 * @brief draw_current_line
 * @param img
 * @param p1
 * @param p2
 */
void draw_current_line(cv::Mat& img,cv::Point p1,cv::Point p2)
{
    cv::LineIterator it(img,p1,p2,8);
    for(int i = 0; i < it.count; i++,it++)
    {
        cv::Point curr = it.pos();
        if(img.at<cv::Vec3b>(curr)[0]>100
                &&  img.at<cv::Vec3b>(curr)[1]>100
                && img.at<cv::Vec3b>(curr)[2]>100)
        {
            /*img.at<cv::Vec3b>(curr)[0]  = 0;
            img.at<cv::Vec3b>(curr)[1]  = 0;
            img.at<cv::Vec3b>(curr)[2]  = 255;*/
            circle(img,curr,2,Scalar(0,0,255,0.2),CV_FILLED,8,0);
        }
    }
}


/**
 * @brief make_line
 * @param points
 * @return
 */
inline
int make_line(std::list<vint2> &points)
{
    std::vector<int> vect_1;
    std::vector<int> vect_0;
    int i = 0,j = 0;
    for(auto &l : points)
    {
        if(i==0 || i==int(2*points.size()/3) )
        {
            vect_1.push_back(l[1]);
            vect_0.push_back(l[0]);
        }
        i++;
    }
    int val1 = vect_1[1]-vect_1[0];
    int val0 = vect_0[1]-vect_0[0];
    if(val1>val0)
    {
        points.sort([](const vint2 & a, const vint2 & b) { return a[1] > b[1]; });
        return 1;
    }
    else
    {
        points.sort([](const vint2 & a, const vint2 & b) { return a[0] > b[0]; });
        return 0;
    }
}

template <typename... OPTS>
/**
 * @brief draw_trajectories_hough_img_ori_tracks_all
 * @param out
 * @param trs
 * @param max_trajectory_len
 * @param T_theta
 * @param nrows
 * @param ncols
 * @param opts
 */
void draw_trajectories_hough_img_ori_tracks_all(cv::Mat & out, std::vector<track>& trs,
                                                int max_trajectory_len,int T_theta,int nrows,int ncols,
                                                OPTS... opts)
{
    auto options = iod::D(opts...);

    /*auto trajectory_color = options.get(_trajectory_color, [] (int i) {
        return vuchar3(255, 255, 0); });*/

    if (max_trajectory_len == -1)
        max_trajectory_len = INT_MAX;

    std::vector<int> sorted_idx;
    for (auto& t : trs) sorted_idx.push_back(sorted_idx.size());

    std::sort(sorted_idx.begin(), sorted_idx.end(), [&] (auto i, auto j) { return trs[i].first_frame() > trs[j].first_frame(); });


    //#pragma omp parallel for
    for (int ti = 0; ti < trs.size(); ti++)
    {
        track& t = trs[sorted_idx[ti]];
        if (!t.isAlive() or t.size() == 0) continue;

        vint2 p1 = t.position_at_frame(t.last_frame());
        vint2 p2 = t.position_at_frame(t.last_frame() - std::min(10, int(t.size()) - 1));

        vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p2[0] - p1[0], p2[1] - p1[1])) * 180.f / M_PI, 1.f, 1.f);

        int last_frame_id = std::max(t.last_frame() - max_trajectory_len, t.first_frame());
        /*if ((p1 - t.position_at_frame(last_frame_id).template cast<int>()).norm() < 4)
            continue;*/
        //cout << "here " << endl;

        if(t.size()<5)
            continue;
        for (int i = t.last_frame(); i >= std::max(t.last_frame() - max_trajectory_len, t.first_frame()) + 1; i--)
        {
            std::list<vint2> list_p1 = t.list_point_at_frame_image(i);
            std::list<vint2> list_p2 = t.list_point_at_frame_image(i - 1);
            vuchar3 color = t.color;
            int inter = max_trajectory_len - i;
            double alpha = (double)inter/(double)max_trajectory_len;
            if(alpha <0.6)
                alpha = 0.6;
            int j = 0;

            for(auto &l : list_p1 )
            {
                //if(j%2==0)
                {
                    float b,g,r;
                    b = (1-alpha)*out.at<cv::Vec3b>(l[0],l[1])[0] + (alpha)*color[0];
                    g = (1-alpha)*out.at<cv::Vec3b>(l[0],l[1])[1] + (alpha)*color[1];
                    r = (1-alpha)*out.at<cv::Vec3b>(l[0],l[1])[2] + (alpha)*color[2];
                    out.at<cv::Vec3b>(l[0],l[1])[0]  = uchar(b);
                    out.at<cv::Vec3b>(l[0],l[1])[1]  = uchar(g);
                    out.at<cv::Vec3b>(l[0],l[1])[2]  = uchar(r);
                }
                j++;
            }
            j = 0;
            for(auto &l : list_p2 )
            {
                //if(j%2==0)
                {
                    float b,g,r;
                    b = (1-alpha)*out.at<cv::Vec3b>(l[0],l[1])[0] + (alpha)*color[0];
                    g = (1-alpha)*out.at<cv::Vec3b>(l[0],l[1])[1] + (alpha)*color[1];
                    r = (1-alpha)*out.at<cv::Vec3b>(l[0],l[1])[2] + (alpha)*color[2];
                    out.at<cv::Vec3b>(l[0],l[1])[0]  = uchar(b);
                    out.at<cv::Vec3b>(l[0],l[1])[1]  = uchar(g);
                    out.at<cv::Vec3b>(l[0],l[1])[2]  = uchar(r);
                }
                j++;
            }
        }
        std::list<vint2> list_pf = t.last_list_points;
        make_line(list_pf);
        int i = 0,j;
        std::vector<vint2> list_ligne;
        for(auto &l : list_pf)
        {
            list_ligne.push_back(l);
        }
        for(int i = 0; i < list_ligne.size()-1;i++)
        {
            float nor = (list_ligne[i] - list_ligne[i+1]).norm();
            if( nor < 2 || nor > 5 )
            {
                circle(out,cv::Point((list_ligne[i])[1],(list_ligne[i])[0]),2,Scalar(t.color[0],t.color[1],t.color[2]),CV_FILLED,8,0);
                circle(out,cv::Point((list_ligne[i+1])[1],(list_ligne[i+1])[0]),2,Scalar(t.color[0],t.color[1],t.color[2]),CV_FILLED,8,0);
            }
            else if(nor<5)
            {
                cv::line(out, cv::Point((list_ligne[i])[1],(list_ligne[i])[0]),
                        cv::Point((list_ligne[i+1])[1],(list_ligne[i+1])[0]), Scalar(t.color[0],t.color[1],t.color[2]),2);
            }
        }
    }
}

template <typename... OPTS>
/**
 * @brief draw_trajectories_hough_img_ori_tracks_all1
 * @param out
 * @param trs
 * @param max_trajectory_len
 * @param T_theta
 * @param nrows
 * @param ncols
 * @param opts
 */
void draw_trajectories_hough_img_ori_tracks_all1(cv::Mat & out, std::vector<track>& trs,
                                                 int max_trajectory_len,int T_theta,int nrows,int ncols,
                                                 OPTS... opts)
{
    auto options = iod::D(opts...);

    /*auto trajectory_color = options.get(_trajectory_color, [] (int i) {
        return vuchar3(255, 255, 0); });*/

    if (max_trajectory_len == -1)
        max_trajectory_len = INT_MAX;

    std::vector<int> sorted_idx;
    for (auto& t : trs) sorted_idx.push_back(sorted_idx.size());

    std::sort(sorted_idx.begin(), sorted_idx.end(), [&] (auto i, auto j) { return trs[i].first_frame() > trs[j].first_frame(); });


    //#pragma omp parallel for
    for (int ti = 0; ti < trs.size(); ti++)
    {
        track& t = trs[sorted_idx[ti]];
        if (!t.isAlive() or t.size() == 0) continue;

        vint2 p1 = t.position_at_frame(t.last_frame());
        vint2 p2 = t.position_at_frame(t.last_frame() - std::min(10, int(t.size()) - 1));

        vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p2[0] - p1[0], p2[1] - p1[1])) * 180.f / M_PI, 1.f, 1.f);

        int last_frame_id = std::max(t.last_frame() - max_trajectory_len, t.first_frame());
        /*if ((p1 - t.position_at_frame(last_frame_id).template cast<int>()).norm() < 4)
            continue;*/
        //cout << "here " << endl;

        if(t.size()<5)
            continue;
        for (int i = t.last_frame(); i >= std::max(t.last_frame() - max_trajectory_len, t.first_frame()) + 1; i--)
        {
            vint2 p1 = t.position_at_frame(i);
            vint2 p2 = t.position_at_frame(i - 1);
            //vint4 ori1 = getLineFromPoint(p1[0],p1[1],T_theta,nrows,ncols);
            vuchar3 color = t.color;
            int inter = max_trajectory_len - i;

        }
        vint2 lp = t.last_point;
        float cosinus,sinus;
        vint4 ori = getLineFromPoint_Fast(lp[0],lp[1],T_theta,nrows,ncols,cosinus,sinus);
        //vint4 ori = getLineFromPoint(lp[0],lp[1],T_theta,nrows,ncols);
        cv::line(out, cv::Point(ori[0],ori[1]), cv::Point(ori[2],ori[3]), Scalar(t.color[0],t.color[1],t.color[2]),3);
    }
}

template <typename... OPTS>
/**
 * @brief draw_trajectories_hough_img_ori_traj
 * @param out
 * @param trs
 * @param max_trajectory_len
 * @param T_theta
 * @param nrows
 * @param ncols
 * @param opts
 */
void draw_trajectories_hough_img_ori_traj(cv::Mat & out, std::vector<keypoint_trajectory>& trs,
                                          int max_trajectory_len,int T_theta,int nrows,int ncols,
                                          OPTS... opts)
{
    auto options = iod::D(opts...);

    auto trajectory_color = options.get(_trajectory_color, [] (int i) {
        return vuchar3(255, 255, 0); });

    if (max_trajectory_len == -1)
        max_trajectory_len = INT_MAX;

    std::vector<int> sorted_idx;
    for (auto& t : trs) sorted_idx.push_back(sorted_idx.size());

    std::sort(sorted_idx.begin(), sorted_idx.end(), [&] (auto i, auto j) { return trs[i].start_frame() > trs[j].start_frame(); });

#pragma omp parallel for
    for (int ti = 0; ti < trs.size(); ti++)
    {
        keypoint_trajectory& t = trs[sorted_idx[ti]];
        if (!t.alive() or t.size() == 0) continue;

        vint2 p1 = t.position_at_frame(t.end_frame()).template cast<int>();
        vint2 p2 = t.position_at_frame(t.end_frame() - std::min(10, int(t.size()) - 1)).template cast<int>();

        //vuchar3 pt_color = hsv_to_rgb((M_PI + atan2(p2[0] - p1[0], p2[1] - p1[1])) * 180.f / M_PI, 1.f, 1.f);

        int last_frame_id = std::max(t.end_frame() - max_trajectory_len, t.start_frame());
        if ((p1 - t.position_at_frame(last_frame_id).template cast<int>()).norm() < 4)
            continue;

        for (int i = t.end_frame(); i >= std::max(t.end_frame() - max_trajectory_len, t.start_frame()) + 1; i--)
        {
            vint2 p1 = t.position_at_frame(i).template cast<int>();
            vint2 p2 = t.position_at_frame(i - 1).template cast<int>();

            vint4 l1 = getLineFromPoint(p1[0],p1[1],T_theta,nrows,ncols);
            vint4 l2 = getLineFromPoint(p2[0],p2[1],T_theta,nrows,ncols);

            //vector<cv::Point> points(4);
            //points = { (x1,y1),(x2,y2),(x3,y3),(x4,y4) };

            //std::cout << " de : (" << p1[0] << "," << p1[1] << ")  à  (" << p2[0]  << "," << p2[1] << ")" << std::endl;
            vuchar4 color;
            /*color.segment<3>(0) = pt_color;
            color[3] = 0.4f*(255.f - 255.f * (t.end_frame() - i) / max_trajectory_len);*/
            color = vuchar4(0,255,0,255);

            cv::line(out, cv::Point(l1[0],l1[1]), cv::Point(l1[2],l1[3]), Scalar(0,255,0,0.4),10);
            cv::line(out, cv::Point(l2[0],l2[1]), cv::Point(l2[2],l2[3]), Scalar(0,255,0,0.4),10);
        }

        //draw::c9(out, t.position().template cast<int>(), pt_color);
        vint2 ori_pol = t.position().template cast<int>();
        vint4 ori_xy = getLineFromPoint(ori_pol[0],ori_pol[1],T_theta,nrows,ncols);
        cv::line(out, cv::Point(ori_xy[0],ori_xy[1]), cv::Point(ori_xy[2],ori_xy[3]), Scalar(0,0,255),2);

    }
}

}
