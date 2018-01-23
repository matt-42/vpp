
#include "track.hh"
#include "vpp/algorithms/line_tracker_4_sfm/miscellanous.hh"


namespace vpp
{
track::track(vint2 pt, int id, int max_tr, float grd, int id_of_frame, float motion_threshold, int wkf)
{
    this->id_track = id;
    this->max_trajectory_size = max_tr;
    this->alive = true;
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->start_frame = id_of_frame;
    this->age = -1;
    this->frame_without_update = 0;
    this->motion_threshold_track = motion_threshold;
    this->use_kf = wkf;
    this->color = generate_color();
    if(this->use_kf)
    {
        this->ukf = std::make_unique<unscented_kalman_filter>();
    }
    add_new_point(pt);
}

track::track(vint2 pt, int id, int max_tr, float grd, int id_of_frame, float motion_threshold,
             int wkf , image2d<uchar> grad_img)
{
    this->id_track = id;
    this->max_trajectory_size = max_tr;
    this->alive = true;
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->start_frame = id_of_frame;
    this->age = -1;
    this->frame_without_update = 0;
    this->motion_threshold_track = motion_threshold;
    this->use_kf = wkf;
    this->color = generate_color();
    if(this->use_kf)
    {
        this->ukf = std::make_unique<unscented_kalman_filter>();
    }
    add_new_point(pt,grad_img);
}


track::track(vint2 pt, vint4 pimg, int id, int max_tr, float grd, int id_of_frame, float motion_threshold)
{
    this->id_track = id;
    this->max_trajectory_size = max_tr;
    this->alive = true;
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->start_frame = id_of_frame;
    this->age = -1;
    this->frame_without_update = 0;
    this->motion_threshold_track = motion_threshold;
    this->color = generate_color();
    add_new_point(pt,pimg);
}

track::track(vint2 pt, std::list<vint2> list_p, int id, int max_tr, float grd, int id_of_frame, float motion_threshold)
{
    this->id_track = id;
    this->max_trajectory_size = max_tr;
    this->alive = true;
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->start_frame = id_of_frame;
    this->age = -1;
    this->frame_without_update = 0;
    this->motion_threshold_track = motion_threshold;
    this->color = generate_color();
    add_new_point(pt,list_p);
}

void track::add_new_point(vint2 pt)
{
    this->age++;
    this->positions.push_back(pt);
    this->fil_ariane.push_back(1);
    this->last_point = pt;
    this->frame_without_update = 0;
    int taille = this->positions.size();
    if(taille>=3)
    {
        drho = ((this->positions[taille-1])[0] - (this->positions[taille-3])[0]) /2.0;
        dtheta = ((this->positions[taille-1])[1] - (this->positions[taille-3])[1]) /2.0;
        float t = sqrt(pow(2*drho,2)+pow(2*dtheta,2));
        if(t>0)
        this->motion_threshold_track = 4*t;
        //cout << "seuil " << this->motion_threshold_track  << endl;
    }

    while(this->positions.size()>this->max_trajectory_size)
    {
        this->pop_front<vint2>(this->positions);
        this->pop_front<int>(this->fil_ariane);
    }
    if(this->use_kf)
    {
        ukf->add_new_dectection(pt,0.33);
    }
}

void track::add_new_point(vint2 pt,image2d<uchar> grad_img)
{
    this->age++;
    image2d<uchar> p_grad(grad_img.domain());
    vpp::copy(grad_img,p_grad);
    this->list_grad_img.push_back(p_grad);
    this->positions.push_back(pt);
    this->fil_ariane.push_back(1);
    this->last_point = pt;
    this->frame_without_update = 0;
    int taille = this->positions.size();
    if(taille>=3)
    {
        drho = ((this->positions[taille-1])[0] - (this->positions[taille-3])[0]) /2.0;
        dtheta = ((this->positions[taille-1])[1] - (this->positions[taille-3])[1]) /2.0;
        float t = sqrt(pow(2*drho,2)+pow(2*dtheta,2));
        if(t>0)
        this->motion_threshold_track = 4*t;
        //cout << "seuil " << this->motion_threshold_track  << endl;
    }

    while(this->positions.size()>this->max_trajectory_size)
    {
        this->pop_front<vint2>(this->positions);
        this->pop_front<int>(this->fil_ariane);
        this->pop_front<image2d<uchar>>(this->list_grad_img);
    }
    if(this->use_kf)
    {
        ukf->add_new_dectection(pt,0.33);
    }
}

void track::only_update_trajectory()
{
    if(this->use_kf)
    {
        this->fil_ariane.push_back(-1);
        this->age++;
        this->ukf->only_update_track(0.33);
        vint2 pt = vint2(int(this->ukf->state_vector[0]),int(this->ukf->state_vector[1]));
        this->positions.push_back(pt);
        this->last_point = pt;
        int taille = this->positions.size();
        if(taille>=3)
        {
            drho = ((this->positions[taille-1])[0] - (this->positions[taille-3])[0]) /2.0;
            dtheta = ((this->positions[taille-1])[1] - (this->positions[taille-3])[1]) /2.0;
            float t = sqrt(pow(2*drho,2)+pow(2*dtheta,2));
            if(t>0)
            this->motion_threshold_track = 4*t;
        }

        while(this->positions.size()>this->max_trajectory_size)
        {
            this->pop_front<vint2>(this->positions);
            this->pop_front<int>(this->fil_ariane);
        }
    }
}

void track::add_new_point(vint2 pt,vint4 pimg)
{
    this->age++;
    this->positions.push_back(pt);
    this->pos_img.push_back(pimg);
    this->last_point = pt;
    this->last_point_xy = pimg;
    this->frame_without_update = 0;
    int taille = this->positions.size();
    if(taille>=3)
    {
        drho = ((this->positions[taille])[0] - (this->positions[taille-2])[0]) /2.0;
        dtheta = ((this->positions[taille])[1] - (this->positions[taille-2])[1]) /2.0;
    }
    this->motion_threshold_track = 2*sqrt(pow(2*drho,2)+pow(2*dtheta,2));
    while(this->positions.size()>this->max_trajectory_size)
    {
        this->pop_front<vint4>(this->pos_img);
        this->pop_front<vint2>(this->positions);
    }
}


void track::add_new_point(vint2 pt,std::list<vint2> list_p)
{
    this->age++;
    this->positions.push_back(pt);
    this->list_points.push_back(list_p);
    this->last_point = pt;
    this->last_list_points = list_p;
    this->frame_without_update = 0;
    int taille = this->positions.size();
    if(taille>=3)
    {
        drho = ((this->positions[taille])[0] - (this->positions[taille-2])[0]) /2.0;
        dtheta = ((this->positions[taille])[1] - (this->positions[taille-2])[1]) /2.0;
    }
    this->motion_threshold_track = 2*sqrt(pow(2*drho,2)+pow(2*dtheta,2));
    while(this->positions.size()>this->max_trajectory_size)
    {
        this->pop_front<std::list<vint2>>(this->list_points);
        this->pop_front<vint2>(this->positions);
    }
}

void track::add_new_point(vint2 pt, float grd, int id_of_frame)
{
    this->add_new_point(pt);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}

void track::add_new_point(vint2 pt, float grd, int id_of_frame,image2d<uchar> grad_img)
{
    this->add_new_point(pt,grad_img);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}

void track::add_new_point(vint2 pt, vint4 pimg, float grd, int id_of_frame)
{
    this->add_new_point(pt,pimg);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}

void track::add_new_point(vint2 pt,std::list<vint2> list_p, float grd, int id_of_frame)
{
    this->add_new_point(pt,list_p);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}



}


