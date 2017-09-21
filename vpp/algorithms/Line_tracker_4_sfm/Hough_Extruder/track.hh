#ifndef TRACK_HH
#define TRACK_HH

#include <vpp/vpp.hh>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <list>
#include "vpp/algorithms/Line_tracker_4_sfm/Hough_Extruder/unscented_kalman_filter.hh"

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
    std::vector<image2d<uchar>> list_grad_img;
    std::vector<int> fil_ariane;
    std::vector<vint4> pos_img;
    std::vector<std::list<vint2>> list_points;
    std::list<vint2> last_list_points;
    int use_kf;
    std::unique_ptr<Unscented_Kalman_Filter> ukf;
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
    void addNewPoint(vint2 pt);
    void addNewPoint(vint2 pt, image2d<uchar> grad_img);
    void addNewPoint(vint2 pt, vint4 pimg);
    void addNewPoint(vint2 pt,std::list<vint2> list_p);
    void addNewPoint(vint2 pt,float grd,int id_of_frame);
    void addNewPoint(vint2 pt,float grd,int id_of_frame, image2d<uchar> grad_img);
    void addNewPoint(vint2 pt,vint4 pimg,float grd,int id_of_frame);
    void addNewPoint(vint2 pt,std::list<vint2> list_p, float grd, int id_of_frame);
    void onlyUpdateTrajectory();

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

#endif // TRACK_HH
