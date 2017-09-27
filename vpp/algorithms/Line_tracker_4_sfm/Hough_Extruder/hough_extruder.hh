#ifndef HOUGH_EXTRUDER_HH
#define HOUGH_EXTRUDER_HH

#include <ctime>
#include <cstdlib>

#include <iod/parse_command_line.hh>
#include <iod/timer.hh>

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <vpp/vpp.hh>
#include <Eigen/Core>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/video_extruder.hh>
#include <vpp/utils/opencv_utils.hh>
#include <vpp/draw/draw_trajectories.hh>
#include <vpp/algorithms/Line_tracker_4_sfm/Hough_Extruder/draw_trajectories_hough.hh>
#include "vpp/algorithms/Line_tracker_4_sfm/symbols.hh"
#include "feature_matching_hough.hh"
#include "paint.hh"


using namespace vpp;
using namespace vppx;
using namespace std;
using namespace Eigen;
using namespace iod;
using namespace cv;

namespace vpp{

typedef vpp::keypoint_container<keypoint<int>, int> cluster_container_int;
typedef vpp::keypoint_container<keypoint<float>, float> cluster_container_float;

struct foreach_videoframe1
{

    foreach_videoframe1(const char* f)
    {
        open_videocapture(f, cap_);
        frame_ = vpp::image2d<vpp::vuchar3>(videocapture_domain(cap_));
        cvframe_ = to_opencv(frame_);
    }

    template <typename F>
    void operator| (F f)
    {
        while (cap_.read(cvframe_))
        {
            f(frame_);
            //cout << "taille " << cvframe_.cols << endl;
        }
    }

    template <typename C>
    void operator/ (C c)
    {
        while (cap_.read(cvframe_))
        {
            c = cvframe_;
        }
    }

private:
    cv::Mat cvframe_;
    vpp::image2d<vpp::vuchar3> frame_;
    cv::VideoCapture cap_;
};

struct foreach_videoframe2
{

    foreach_videoframe2(const char* f)
    {
        open_videocapture(f, cap_);
        frame_ = vpp::image2d<vpp::vuchar3>(videocapture_domain(cap_));
        cvframe_ = to_opencv(frame_);
    }

    template <typename F>
    void operator| (F f)
    {
        while (cap_.read(cvframe_))
        {
            f(frame_);
        }
    }

    template <typename C>
    void operator/ (C c)
    {
        while (cap_.read(cvframe_))
        {
            c = cvframe_;
        }
    }

private:
    cv::Mat cvframe_;
    vpp::image2d<vpp::vuchar3> frame_;
    cv::VideoCapture cap_;
};

struct foreach_frame
{

    foreach_frame(const char* f)
    {
        open_videocapture(f, cap_);
        frame_ = vpp::image2d<vpp::vuchar3>(videocapture_domain(cap_));
        cvframe_ = to_opencv(frame_);
    }

    template <typename F>
    void operator| (F f)
    {
        while (cap_.read(cvframe_)){ f(from_opencv<vuchar3>(cvframe_));
            cout << "ben voyons "<< endl;
            //cv::imwrite("see.bmp", to_opencv(f));
        }
    }

private:
    cv::Mat cvframe_;
    vpp::image2d<vpp::vuchar3> frame_;
    cv::VideoCapture cap_;
};

template <typename... OPTS>
void hough_feature_matching_in_n_frames(int T_theta,const char* link,
                                        Sclare_rho scale,
                                        Type_video_hough type_video,
                                        Type_output type_sortie,
                                        Type_Lines type_line,
                                        Frequence freq,
                                        int wkf,int N,
                                        OPTS... options)
{

    auto opts = D(options...);
    const int detector_th = opts.get(_detector_th, 10);
    const int keypoint_spacing = opts.get(_keypoint_spacing, 10);
    const int detector_period = opts.get(_detector_period, 5);
    const int max_trajectory_length = opts.get(_max_trajectory_length, 15);
    const int slot_hough = opts.get(_slot_hough, 5);
    const int m_first_lines = opts.get(_m_first_lines, 10);
    const float acc_threshold = opts.get(_acc_threshold, 100);

    ofstream myfile;
    myfile.open ("res.txt");

    bool first = true;
    int nframes = 0;
    int id_trackers = 0;
    //MultiPointTracker multitracking(vint2(0,0),20, 0.1,10, 0.2,0.5,20, 0.1,2);
    cv::VideoWriter output_video;
    /**************************************/
    std::vector<float> prev_acc;
    std::vector<float> _acc_gl;
    /*************************************/
    std::vector<vint2> old_vects;
    /*************************************/
    image2d<uchar> frame_gradient(make_box2d(1,1));
    image2d<uchar> frame_point(make_box2d(1,1));
vpp:feature_matching_hough_ctx ctx= feature_matching_hough_init(make_box2d(1,1));
    int modulo_cp = 0;
    int us_cpt = 0;
    timer gt;
    gt.start();
    std::list<image2d<uchar>> liste_image;
    foreach_videoframe1(link)| [&] (const image2d<vuchar3>& frame_cv){
        if(nframes>=0 && nframes<600)
        {
            timer t;
            t.start();
            auto frame = rgb_to_graylevel<uchar>(frame_cv);
            image2d<uchar> frame_img(frame.domain());
            gaussian_blur5(frame,frame_img);
            int ncols = frame_img.ncols();
            int nrows = frame_img.nrows();
            int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
            liste_image.push_back(frame_img);
            if(first)
            {
                if(Type_output::VIDEO_HOUGH == type_sortie)
                {
                    output_video.open("videos/output/hough_video.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30.f,
                                      cv::Size(T_theta,rhomax), true);
                }
                else if(Type_output::ORIGINAL_VIDEO == type_sortie || Type_output::GRADIENT_VIDEO == type_sortie)
                {
                    output_video.open("videos/output/original_video.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30.f,
                                      cv::Size(ncols,nrows), true);
                }
                std::vector<float> prev_acc_temp(rhomax*T_theta,0);
                std::vector<float> _acc_gl_temp(rhomax*T_theta,0);

                image2d<uchar> frame_gradient_temp(make_box2d(nrows,ncols));
                frame_gradient = frame_gradient_temp;

                image2d<uchar> frame_point_temp(make_box2d(rhomax,T_theta));
                frame_point = frame_point_temp;

                prev_acc = prev_acc_temp;
                _acc_gl = _acc_gl_temp;

                feature_matching_hough_update_three_first(ctx, prev_acc, _acc_gl,frame_gradient,frame_point,scale,
                                                          int(type_video),frame_img,T_theta,rhomax,first, type_sortie,
                                                          id_trackers,type_line,freq,old_vects,wkf,
                                                          _detector_th = detector_th,
                                                          _slot_hough = slot_hough,
                                                          _keypoint_spacing = keypoint_spacing,
                                                          _detector_period = detector_period,
                                                          _m_first_lines = m_first_lines,
                                                          _max_trajectory_length = max_trajectory_length,
                                                          _acc_threshold = acc_threshold);


                first = false;
            }
            else if(nframes%N == 0)
            {
                feature_matching_hough_update_three_first(ctx, prev_acc, _acc_gl,frame_gradient,frame_point,scale,
                                                          int(type_video),frame_img,T_theta,rhomax,first, type_sortie,
                                                          id_trackers, type_line,freq,old_vects, wkf,
                                                          _detector_th = detector_th,
                                                          _slot_hough = slot_hough,
                                                          _keypoint_spacing = keypoint_spacing,
                                                          _detector_period = detector_period,
                                                          _m_first_lines = m_first_lines,
                                                          _max_trajectory_length = max_trajectory_length,
                                                          _acc_threshold = acc_threshold);
            }

            t.end();


            us_cpt += t.us();
            if (!(nframes%10))
            {
                std::cout << "Tracker time: " << (us_cpt / 10000.f) << " ms/frame. " << ctx.list_track.size() << " particles." << std::endl;
                us_cpt = 0;
            }
            cout << " frame no " << nframes << endl;


            if(Type_output::VIDEO_HOUGH == type_sortie || Type_output::GRADIENT_VIDEO == type_sortie)
            {

                if(Type_output::GRADIENT_VIDEO == type_sortie)
                {
                    auto display = graylevel_to_rgb<vuchar3>(frame_gradient);
                    cv::Mat mat_ori = to_opencv(display);
                    if(Type_Lines::ALL_POINTS==type_line)
                    {
                        draw_trajectories_hough_img_ori_tracks_all(mat_ori, ctx.list_track,
                                                                   max_trajectory_length,T_theta,nrows,ncols);
                    }
                    else if(Type_Lines::EXTREMITE==type_line)
                    {
                        draw_trajectories_hough_img_ori_tracks(mat_ori, ctx.list_track,
                                                               max_trajectory_length,T_theta,nrows,ncols);
                    }
                    else if(Type_Lines::ONLY_POLAR==type_line)
                    {
                        draw_trajectories_hough_img_ori_tracks_all1(mat_ori, ctx.list_track,
                                                                    max_trajectory_length,T_theta,nrows,ncols);
                    }

                    if (output_video.isOpened())
                        output_video << mat_ori;
                }
                else
                {
                    auto display = graylevel_to_rgb<vuchar3>(frame_point);
                    draw_trajectories_hough_V1(display, ctx.list_track, max_trajectory_length);
                    if (output_video.isOpened())
                        output_video << to_opencv(display);
                }
            }
            else if(Type_output::ORIGINAL_VIDEO == type_sortie)
            {
                image2d<vuchar3> temp_frame(make_box2d(nrows,ncols));
                vpp::copy(frame_cv,temp_frame);
                cv::Mat mat_ori = to_opencv(temp_frame);
                if(Type_Lines::ALL_POINTS==type_line)
                {
                    draw_trajectories_hough_img_ori_tracks_all(mat_ori, ctx.list_track,
                                                               max_trajectory_length,T_theta,nrows,ncols);
                }
                else if(Type_Lines::EXTREMITE==type_line)
                {
                    draw_trajectories_hough_img_ori_tracks(mat_ori, ctx.list_track,
                                                           max_trajectory_length,T_theta,nrows,ncols);
                }
                else if(Type_Lines::ONLY_POLAR==type_line)
                {
                    draw_trajectories_hough_img_ori_tracks_all1(mat_ori, ctx.list_track,
                                                                max_trajectory_length,T_theta,nrows,ncols);
                }
                if (output_video.isOpened())
                    output_video << mat_ori;
            }

        }
        modulo_cp++;
        modulo_cp = modulo_cp % N;
        nframes++;
    };
    gt.end();

    cout << "temps hy " << us_cpt/1000.0 << endl;
}

template <typename... OPTS>
void hough_feature_matching(int T_theta,const char* link,
                            Sclare_rho scale,
                            Type_video_hough type_video,
                            Type_output type_sortie,
                            Type_Lines type_line,
                            Frequence freq,
                            int wkf,With_Entries we,
                            OPTS... options)
{

    auto opts = D(options...);
    const int detector_th = opts.get(_detector_th, 10);
    const int keypoint_spacing = opts.get(_keypoint_spacing, 10);
    const int detector_period = opts.get(_detector_period, 5);
    const int max_trajectory_length = opts.get(_max_trajectory_length, 15);
    const int slot_hough = opts.get(_slot_hough, 5);
    const int m_first_lines = opts.get(_m_first_lines, 10);
    const float acc_threshold = opts.get(_acc_threshold, 100);

    ofstream myfile;
    myfile.open ("res.txt");

    bool first = true;
    int nframes = 0;
    int id_trackers = 0;
    //MultiPointTracker multitracking(vint2(0,0),20, 0.1,10, 0.2,0.5,20, 0.1,2);
    cv::VideoWriter output_video;
    /**************************************/
    std::vector<float> prev_acc;
    std::vector<float> _acc_gl;
    /*************************************/
    std::vector<vint2> old_vects;
    /*************************************/
    image2d<uchar> frame_gradient(make_box2d(1,1));
    image2d<uchar> frame_point(make_box2d(1,1));
vpp:feature_matching_hough_ctx ctx= feature_matching_hough_init(make_box2d(1,1));
    int us_cpt = 0;
    execution_time = 0;
    timer gt;
    gt.start();
    foreach_videoframe1(link)| [&] (const image2d<vuchar3>& frame_cv){
        if(nframes>=0 && nframes<600)
        {
            timer t;
            t.start();
            auto frame = rgb_to_graylevel<uchar>(frame_cv);
            image2d<uchar> frame_img(frame.domain());
            gaussian_blur5(frame,frame_img);
            int ncols = frame_img.ncols();
            int nrows = frame_img.nrows();
            int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
            if(first)
            {
                if(Type_output::VIDEO_HOUGH == type_sortie)
                {
                    output_video.open("videos/output/hough_video.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30.f,
                                      cv::Size(T_theta,rhomax), true);
                }
                else if(Type_output::ORIGINAL_VIDEO == type_sortie || Type_output::GRADIENT_VIDEO == type_sortie)
                {
                    output_video.open("videos/output/original_video.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30.f,
                                      cv::Size(ncols,nrows), true);
                }
                std::vector<float> prev_acc_temp(rhomax*T_theta,0);
                std::vector<float> _acc_gl_temp(rhomax*T_theta,0);

                image2d<uchar> frame_gradient_temp(make_box2d(nrows,ncols));
                frame_gradient = frame_gradient_temp;

                image2d<uchar> frame_point_temp(make_box2d(rhomax,T_theta));
                frame_point = frame_point_temp;

                prev_acc = prev_acc_temp;
                _acc_gl = _acc_gl_temp;

                feature_matching_hough_update_three_first(ctx, prev_acc, _acc_gl,frame_gradient,frame_point,scale,
                                                          int(type_video),frame_img,T_theta,rhomax,first, type_sortie,
                                                          id_trackers,type_line,freq,old_vects,wkf,we,
                                                          _detector_th = detector_th,
                                                          _slot_hough = slot_hough,
                                                          _keypoint_spacing = keypoint_spacing,
                                                          _detector_period = detector_period,
                                                          _m_first_lines = m_first_lines,
                                                          _max_trajectory_length = max_trajectory_length,
                                                          _acc_threshold = acc_threshold);


                first = false;
            }
            else
            {
                feature_matching_hough_update_three_first(ctx, prev_acc, _acc_gl,frame_gradient,frame_point,scale,
                                                          int(type_video),frame_img,T_theta,rhomax,first, type_sortie,
                                                          id_trackers, type_line,freq,old_vects, wkf,we,
                                                          _detector_th = detector_th,
                                                          _slot_hough = slot_hough,
                                                          _keypoint_spacing = keypoint_spacing,
                                                          _detector_period = detector_period,
                                                          _m_first_lines = m_first_lines,
                                                          _max_trajectory_length = max_trajectory_length,
                                                          _acc_threshold = acc_threshold);
            }

            t.end();


            us_cpt += t.us();
            if (!(nframes%10))
            {
                std::cout << "Tracker time: " << (us_cpt / 10000.f) << " ms/frame. " << ctx.list_track.size() << " particles." << std::endl;
                us_cpt = 0;
            }
            cout << " frame no " << nframes << endl;


            if(Type_output::VIDEO_HOUGH == type_sortie || Type_output::GRADIENT_VIDEO == type_sortie)
            {

                if(Type_output::GRADIENT_VIDEO == type_sortie)
                {
                    auto display = graylevel_to_rgb<vuchar3>(frame_gradient);
                    cv::Mat mat_ori = to_opencv(display);
                    if(Type_Lines::ALL_POINTS==type_line)
                    {
                        draw_trajectories_hough_img_ori_tracks_all(mat_ori, ctx.list_track,
                                                                   max_trajectory_length,T_theta,nrows,ncols);
                    }
                    else if(Type_Lines::EXTREMITE==type_line)
                    {
                        draw_trajectories_hough_img_ori_tracks(mat_ori, ctx.list_track,
                                                               max_trajectory_length,T_theta,nrows,ncols);
                    }
                    else if(Type_Lines::ONLY_POLAR==type_line)
                    {
                        draw_trajectories_hough_img_ori_tracks_all1(mat_ori, ctx.list_track,
                                                                    max_trajectory_length,T_theta,nrows,ncols);
                    }

                    if (output_video.isOpened())
                        output_video << mat_ori;
                }
                else
                {
                    auto display = graylevel_to_rgb<vuchar3>(frame_point);
                    draw_trajectories_hough_V1(display, ctx.list_track, max_trajectory_length);
                    if (output_video.isOpened())
                        output_video << to_opencv(display);
                }
            }
            else if(Type_output::ORIGINAL_VIDEO == type_sortie)
            {
                image2d<vuchar3> temp_frame(make_box2d(nrows,ncols));
                vpp::copy(frame_cv,temp_frame);
                cv::Mat mat_ori = to_opencv(temp_frame);
                if(Type_Lines::ALL_POINTS==type_line)
                {
                    draw_trajectories_hough_img_ori_tracks_all(mat_ori, ctx.list_track,
                                                               max_trajectory_length,T_theta,nrows,ncols);
                }
                else if(Type_Lines::EXTREMITE==type_line)
                {
                    draw_trajectories_hough_img_ori_tracks(mat_ori, ctx.list_track,
                                                           max_trajectory_length,T_theta,nrows,ncols);
                }
                else if(Type_Lines::ONLY_POLAR==type_line)
                {
                    draw_trajectories_hough_img_ori_tracks_all1(mat_ori, ctx.list_track,
                                                                max_trajectory_length,T_theta,nrows,ncols);
                }
                if (output_video.isOpened())
                    output_video << mat_ori;
            }

        }
        nframes++;
    };
    gt.end();

    cout << "temps hy " << us_cpt/1000.0 << endl;
}



template <typename... OPTS>
void hough_feature_matching_paint(int T_theta,const char* link,
                                  Sclare_rho scale,
                                  Type_video_hough type_video,
                                  Type_output type_sortie,
                                  Type_Lines type_line,
                                  Frequence freq,
                                  int wkf,With_Entries we,
                                  OPTS... options)
{

    auto opts = D(options...);
    const int detector_th = opts.get(_detector_th, 10);
    const int keypoint_spacing = opts.get(_keypoint_spacing, 10);
    const int detector_period = opts.get(_detector_period, 5);
    const int max_trajectory_length = opts.get(_max_trajectory_length, 15);
    const int slot_hough = opts.get(_slot_hough, 5);
    const int m_first_lines = opts.get(_m_first_lines, 10);
    const float acc_threshold = opts.get(_acc_threshold, 100);

    ofstream myfile;
    myfile.open ("res.txt");

    bool first = true;
    int nframes = 0;
    int id_trackers = 0;
    //MultiPointTracker multitracking(vint2(0,0),20, 0.1,10, 0.2,0.5,20, 0.1,2);
    cv::VideoWriter output_video;
    /**************************************/
    std::vector<float> prev_acc;
    std::vector<float> _acc_gl;
    /*************************************/
    std::vector<vint2> old_vects;
    /*************************************/
    image2d<uchar> frame_gradient(make_box2d(1,1));
    image2d<uchar> frame_point(make_box2d(1,1));
vpp:feature_matching_hough_ctx ctx= feature_matching_hough_init(make_box2d(1,1));
    int us_cpt = 0;
    execution_time = 0;
    timer gt;
    gt.start();
    image2d<vuchar4> paint_buffer;
    foreach_videoframe1(link)| [&] (const image2d<vuchar3>& frame_cv){
        if(nframes>=0 && nframes<60000)
        {
            timer t;
            t.start();
            auto frame = rgb_to_graylevel<uchar>(frame_cv);
            image2d<uchar> frame_img(frame.domain());
            gaussian_blur5(frame,frame_img);
            int ncols = frame_img.ncols();
            int nrows = frame_img.nrows();
            int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
            if(first)
            {
                if(Type_output::VIDEO_HOUGH == type_sortie)
                {
                    output_video.open("videos/output/hough_video.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30.f,
                                      cv::Size(T_theta,rhomax), true);
                    image2d<vuchar4> temp_paint(make_box2d(rhomax,T_theta));
                    paint_buffer = temp_paint;
                }
                else if(Type_output::ORIGINAL_VIDEO == type_sortie || Type_output::GRADIENT_VIDEO == type_sortie)
                {
                    output_video.open("videos/output/original_video.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30.f,
                                      cv::Size(ncols,nrows), true);
                    image2d<vuchar4> temp_paint(make_box2d(nrows,ncols));
                    paint_buffer = temp_paint;
                }

                fill(paint_buffer, vuchar4(0,0,0,0));
                std::vector<float> prev_acc_temp(rhomax*T_theta,0);
                std::vector<float> _acc_gl_temp(rhomax*T_theta,0);

                image2d<uchar> frame_gradient_temp(make_box2d(nrows,ncols));
                frame_gradient = frame_gradient_temp;

                image2d<uchar> frame_point_temp(make_box2d(rhomax,T_theta));
                frame_point = frame_point_temp;

                prev_acc = prev_acc_temp;
                _acc_gl = _acc_gl_temp;

                feature_matching_hough_update_three_first(ctx, prev_acc, _acc_gl,frame_gradient,frame_point,scale,
                                                          int(type_video),frame_img,T_theta,rhomax,first, type_sortie,
                                                          id_trackers,type_line,freq,old_vects,wkf,we,
                                                          _detector_th = detector_th,
                                                          _slot_hough = slot_hough,
                                                          _keypoint_spacing = keypoint_spacing,
                                                          _detector_period = detector_period,
                                                          _m_first_lines = m_first_lines,
                                                          _max_trajectory_length = max_trajectory_length,
                                                          _acc_threshold = acc_threshold);


                first = false;
            }
            else
            {
                feature_matching_hough_update_three_first(ctx, prev_acc, _acc_gl,frame_gradient,frame_point,scale,
                                                          int(type_video),frame_img,T_theta,rhomax,first, type_sortie,
                                                          id_trackers, type_line,freq,old_vects, wkf,we,
                                                          _detector_th = detector_th,
                                                          _slot_hough = slot_hough,
                                                          _keypoint_spacing = keypoint_spacing,
                                                          _detector_period = detector_period,
                                                          _m_first_lines = m_first_lines,
                                                          _max_trajectory_length = max_trajectory_length,
                                                          _acc_threshold = acc_threshold);
            }

            t.end();





            us_cpt += t.us();
            if (!(nframes%10))
            {
                std::cout << "Tracker time: " << (us_cpt / 10000.f) << " ms/frame. " << ctx.list_track.size() << " particles." << std::endl;
                us_cpt = 0;
            }

            cout << "Frame no " << nframes << endl;

            if(Type_output::VIDEO_HOUGH == type_sortie || Type_output::GRADIENT_VIDEO == type_sortie)
            {
                if(Type_output::GRADIENT_VIDEO == type_sortie)
                {
                    auto display = graylevel_to_rgb<vuchar3>(frame_gradient);
                    paint_original_video(ctx.list_track, paint_buffer,T_theta,nrows,ncols);
                    pixel_wise(paint_buffer, display) | [] (auto p, auto& d)
                    {
                        d = ((d.template cast<int>() * (255 - p[3]) + p.template segment<3>(0).template cast<int>() * p[3]) / 255).template cast<unsigned char>();
                    };
                    if (output_video.isOpened())
                        output_video << to_opencv(display);
                }
                else
                {
                    auto display = graylevel_to_rgb<vuchar3>(frame_point);
                    paint_hough_video(ctx.list_track, paint_buffer);
                    pixel_wise(paint_buffer, display) | [] (auto p, auto& d)
                    {
                        d = ((d.template cast<int>() * (255 - p[3]) + p.template segment<3>(0).template cast<int>() * p[3]) / 255).template cast<unsigned char>();
                    };
                    if (output_video.isOpened())
                        output_video << to_opencv(display);
                }
            }
            else if(Type_output::ORIGINAL_VIDEO == type_sortie)
            {
                image2d<vuchar3> display(make_box2d(nrows,ncols));
                vpp::copy(frame_cv,display);
                paint_original_video(ctx.list_track, paint_buffer,T_theta,nrows,ncols);
                pixel_wise(paint_buffer, display) | [] (auto p, auto& d)
                {
                    d = ((d.template cast<int>() * (255 - p[3]) + p.template segment<3>(0).template cast<int>() * p[3]) / 255).template cast<unsigned char>();
                };
                if (output_video.isOpened())
                    output_video << to_opencv(display);
            }
        }
        nframes++;
    };

}



template <typename... OPTS>
void Fast_DHT_Matching(int mode, Theta_max discr, Sclare_rho scale,
                   Type_video_hough type_video, Type_output type_sortie,
                   Type_Lines type_line, Frequence freq, With_Kalman_Filter wkf,With_Transparency wt,With_Entries we,
                   OPTS... options)
{

    auto opts = D(options...);
    const int max_trajectory_length = opts.get(_max_trajectory_length, 15);
    const char* link = opts.get(_link_of_video_image, "videos/traindrivers.avi");
    const int rayon_exclusion_theta = opts.get(_rayon_exclusion_theta, 15);
    const int rayon_exclusion_rho = opts.get(_rayon_exclusion_rho, 12);
    const int slot_hough = opts.get(_slot_hough, 5);
    const int m_first_lines = opts.get(_m_first_lines, 10);
    const float acc_threshold = opts.get(_acc_threshold, 100);//
    const float nombre_max_frame_without_update = opts.get(_nombre_max_frame_without_update, 5);

    //char* link = "videos/traindrivers.avi";
    int T_theta = getThetaMax(discr);
    int wkff = getWKF(wkf);

    if(mode==dense_ht)
    {
        hough_image(T_theta,acc_threshold);
    }
    else if(mode == feature_matching)
    {
        cout << "feature_matching_video" << endl;
        if(With_Transparency::YES == wt)
        {
            hough_feature_matching_paint( T_theta,  link,  scale, type_video,type_sortie, type_line,freq,wkff,we,
                                          _max_trajectory_length = max_trajectory_length,
                                          _slot_hough = slot_hough,
                                          _m_first_lines = m_first_lines,
                                          _rayon_exclusion_theta = rayon_exclusion_theta,
                                          _rayon_exclusion_rho = rayon_exclusion_rho,
                                          _acc_threshold = acc_threshold,
                                          _nombre_max_frame_without_update = nombre_max_frame_without_update);
        }
        else
        {
            hough_feature_matching( T_theta,  link,  scale, type_video,type_sortie, type_line,freq,wkff,we,
                                    _max_trajectory_length = max_trajectory_length,
                                    _slot_hough = slot_hough,
                                    _m_first_lines = m_first_lines,
                                    _rayon_exclusion_theta = rayon_exclusion_theta,
                                    _rayon_exclusion_rho = rayon_exclusion_rho,
                                    _acc_threshold = acc_threshold,
                                    _nombre_max_frame_without_update = nombre_max_frame_without_update);
        }

    }
}

}

#endif // HOUGH_EXTRUDER_HH
