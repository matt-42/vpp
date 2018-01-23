#pragma once
#include <vpp/vpp.hh>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <list>
#include "unscented_kalman_filter.hh"

using namespace vpp;
using namespace std;

namespace vpp
{

struct track {

    int id_track;
    int frame_id;
    int start_frame;
    int age;
    int frame_without_update;
    bool alive;
    bool recently_updated;
    std::vector<vint2> positions;
    std::vector<vint2> stabled_positions;
    std::vector<image2d<uchar>> list_grad_img;
    std::vector<int> fil_ariane;
    std::vector<vint4> pos_img;
    std::vector<std::list<vint2>> list_points;
    std::list<vint2> last_list_points;
    int use_kf;
    std::unique_ptr<unscented_kalman_filter> ukf;
    vuchar3 color;
    float drho;
    float dtheta;
    float gradients;
    float motion_threshold_track;
    vint2 last_point;
    vint4 last_point_xy;
    int max_trajectory_size;
    track(vint2 pt, int id_track, int max_tr, float grd, int id_of_frame , float motion_threshold_track, int wkf);
    track(vint2 pt, int id_track, int max_tr, float grd, int id_of_frame ,
          float motion_threshold_track, int wkf, image2d<uchar> grad_img);
    track(vint2 pt,vint4 pimg,int id_track,int max_tr,float grd,int id_of_frame, float motion_threshold_track);
    track(vint2 pt,std::list<vint2> list_p,int id_track,int max_tr,float grd,int id_of_frame, float motion_threshold_track);
    bool isAlive() { return alive; }
    bool isRecentlyUpdated() { return recently_updated; }
    void die() { alive = false; }
    void resurrect() { alive = true; }
    void add_new_point(vint2 pt);
    void add_new_point(vint2 pt, image2d<uchar> grad_img);
    void add_new_point(vint2 pt, vint4 pimg);
    void add_new_point(vint2 pt,std::list<vint2> list_p);
    void add_new_point(vint2 pt,float grd,int id_of_frame);
    void add_new_point(vint2 pt,float grd,int id_of_frame, image2d<uchar> grad_img);
    void add_new_point(vint2 pt,vint4 pimg,float grd,int id_of_frame);
    void add_new_point(vint2 pt,std::list<vint2> list_p, float grd, int id_of_frame);
    void only_update_trajectory();

    template<typename T>
    void pop_front(std::vector<T>& vec)
    {
        assert(!vec.empty());
        vec.erase(vec.begin());
    }


    int size() const
    {
        return positions.size();
    }

    int first_frame() const { return start_frame; }
    int last_frame() const { return start_frame + positions.size() - 1; }

    vint2 position_at_frame(int frame_cpt)
    {
        assert(frame_cpt < (start_frame + positions.size()));
        assert(frame_cpt >= (start_frame));
        int i = positions.size() - 1 - (frame_cpt - start_frame);
        return positions[i];
    }

    image2d<uchar> gradient_image_at_frame(int frame_cpt)
    {
        assert(frame_cpt < (start_frame + list_grad_img.size()));
        assert(frame_cpt >= (start_frame));
        int i = list_grad_img.size() - 1 - (frame_cpt - start_frame);
        return list_grad_img[i];
    }

    vint4 position_at_frame_image(int frame_cpt)
    {
        assert(frame_cpt < (start_frame + pos_img.size()));
        assert(frame_cpt >= (start_frame));
        int i = pos_img.size() - 1 - (frame_cpt - start_frame);
        return pos_img[i];
    }

    std::list<vint2> list_point_at_frame_image(int frame_cpt)
    {
        assert(frame_cpt < (start_frame + list_points.size()));
        assert(frame_cpt >= (start_frame));
        int i = list_points.size() - 1 - (frame_cpt - start_frame);
        return list_points[i];
    }




};


inline
vuchar3 generate_color()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(5,2);
    float h = d(gen)*M_PI;
    float s = d(gen)*M_PI;
    float v = d(gen)*M_PI;
    vuchar3 colr;
    colr = hsv_to_rgb(h, s, v);
    int r,g,b;
    b = (int)colr[0];
    g = (int)colr[1];
    r = (int)colr[2];
    if(r + g + b < 200)
    {
        std::normal_distribution<> dn(10,5);
        b = 10 * dn(gen);
        g = 10 * dn(gen);
        r = 10 * dn(gen);
        b = sign_of_number(b)*b;
        g = sign_of_number(g)*g;
        r = sign_of_number(r)*r;
        colr = vuchar3(b,g,r);
    }
    return colr;
}

struct higher_gradient
{
    inline bool operator() (const track& track1, const track& track2)
    {
        return (track1.gradients > track2.gradients);
    }
};

enum class Type_Line : int16_t { vertical = 0 , r_horizontal = 1 , l_horizontal = 3 } ;


}

#include "track.hpp"


