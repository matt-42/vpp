#ifndef TRACK_HPP
#define TRACK_HPP
#include "track.hh"
#include "vpp/algorithms/Line_tracker_4_sfm/miscellanous/operations.hh"


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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(5,2);
    float h = d(gen)*M_PI;
    float s = d(gen)*M_PI;
    float v = d(gen)*M_PI;
    //cout << " h " << h <<" s " << s << " v " << v << endl;
    color = hsv_to_rgb(h, s, v);
    int r,g,b;
    b = (int)color[0];
    g = (int)color[1];
    r = (int)color[2];
    //cout << (int)color[0] << "   " << (int)color[1] << "   " << (int)color[2] << endl;
    if(r + g + b < 200)
    {
        std::normal_distribution<> dn(10,5);
        b = 10 * dn(gen);
        g = 10 * dn(gen);
        r = 10 * dn(gen);
        b = sign_of_number(b)*b;
        g = sign_of_number(g)*g;
        r = sign_of_number(r)*r;
        color = vuchar3(b,g,r);
    }
    //cout << (int)color[0] << "   " << (int)color[1] << "   " << (int)color[2] << endl;
    if(this->use_kf)
    {
        this->ukf = std::make_unique<Unscented_Kalman_Filter>();
    }
    addNewPoint(pt);
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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(5,2);
    //RNG rng(12345);
    //srand(time(NULL));
    float h = d(gen)*M_PI;
    //srand(time(NULL));
    float s = d(gen)*M_PI;
    //srand(time(NULL));
    float v = d(gen)*M_PI;
    //cout << " h " << h <<" s " << s << " v " << v << endl;
    color = hsv_to_rgb(h, s, v);
    //cout << color[0] << "   " << color[1] << "   " << color[2] << endl;
    if(this->use_kf)
    {
        this->ukf = std::make_unique<Unscented_Kalman_Filter>();
    }
    addNewPoint(pt,grad_img);
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
    RNG rng(12345);
    srand(time(NULL));
    //color = vuchar3(rand()%200+rand()%55,rand()%200+rand()%55,rand()%200+rand()%55);

    if(id%5==0)
    {
        //bleu
        color = vuchar3(rng.uniform(100,255),rng.uniform(0,100),rng.uniform(0,100));
    }
    else if (id%5==1)
    {
        //vert
        color = vuchar3(rng.uniform(0,100),rng.uniform(100,255),rng.uniform(0,100));
    }
    else if (id%5==2)
    {
        //rouge
        color = vuchar3(rng.uniform(0,100),rng.uniform(0,100),rng.uniform(100,255));
    }
    else if (id%5==3)
    {
        //vert
        color = vuchar3(rng.uniform(0,200),rng.uniform(200,255),rng.uniform(0,200));
    }
    else if (id%5==4)
    {
        //rouge
        color = vuchar3(rng.uniform(0,200),rng.uniform(0,200),rng.uniform(200,255));
    }
    addNewPoint(pt,pimg);
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
    RNG rng(12345);
    //color = vuchar3(rand()%200+rand()%55,rand()%200+rand()%55,rand()%200+rand()%55);
    if(id%5==0)
    {
        //bleu
        color = vuchar3(rng.uniform(100,255),rng.uniform(0,100),rng.uniform(0,100));
    }
    else if (id%5==1)
    {
        //vert
        color = vuchar3(rng.uniform(0,255),rng.uniform(100,255),rng.uniform(0,255));
    }
    else if (id%5==2)
    {
        //rouge
        color = vuchar3(rng.uniform(0,100),rng.uniform(0,100),rng.uniform(100,255));
    }
    else if (id%5==3)
    {
        //vert
        color = vuchar3(rng.uniform(0,200),rng.uniform(200,255),rng.uniform(0,200));
    }
    else if (id%5==4)
    {
        //rouge
        color = vuchar3(rng.uniform(0,200),rng.uniform(0,200),rng.uniform(200,255));
    }
    addNewPoint(pt,list_p);
}

void track::addNewPoint(vint2 pt)
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
        ukf->AddNewDectection(pt,0.33);
    }
}

void track::addNewPoint(vint2 pt,image2d<uchar> grad_img)
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
        ukf->AddNewDectection(pt,0.33);
    }
}

void track::onlyUpdateTrajectory()
{
    if(this->use_kf)
    {
        this->fil_ariane.push_back(-1);
        this->age++;
        this->ukf->onlyUpdateTrack(0.33);
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

void track::addNewPoint(vint2 pt,vint4 pimg)
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


void track::addNewPoint(vint2 pt,std::list<vint2> list_p)
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

void track::addNewPoint(vint2 pt, float grd, int id_of_frame)
{
    this->addNewPoint(pt);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}

void track::addNewPoint(vint2 pt, float grd, int id_of_frame,image2d<uchar> grad_img)
{
    this->addNewPoint(pt,grad_img);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}

void track::addNewPoint(vint2 pt, vint4 pimg, float grd, int id_of_frame)
{
    this->addNewPoint(pt,pimg);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}

void track::addNewPoint(vint2 pt,std::list<vint2> list_p, float grd, int id_of_frame)
{
    this->addNewPoint(pt,list_p);
    this->gradients = grd;
    this->frame_id = id_of_frame;
    this->frame_without_update = 0;
}

}


#endif // TRACK_HPP
