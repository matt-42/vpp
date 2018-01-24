

#include "feature_matching_hough.hh"
#include <iod/array_view.hh>
#include <vpp/core/keypoint_trajectory.hh>
#include <vpp/core/keypoint_container.hh>
#include <vpp/core/symbols.hh>
#include <vpp/algorithms/symbols.hh>
#include <vpp/algorithms/optical_flow.hh>
#include <vpp/algorithms/fast_detector/fast.hh>
#include <vpp/algorithms/descriptor_matcher.hh>
#include <chrono>
#include "vpp/algorithms/line_tracker_4_sfm/fast_dht.hh"

/*****************/

/*
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"*/


using namespace std::chrono;
namespace vpp {

// Init the video extruder.
/**
 * @brief feature_matching_hough_init
 * @param domain
 * @return
 */
feature_matching_hough_ctx feature_matching_hough_init(box2d domain)
{
    feature_matching_hough_ctx res(domain);
    res.frame_id = -1;
    return res;
}



/**
 * @brief feature_matching_hough_update_three_first
 * @param ftx
 * @param frame1
 * @param frame2
 * @param frame_grad
 * @param frame_point
 * @param scale
 * @param type_video
 * @param img
 * @param T_theta
 * @param rhomax
 * @param first
 * @param type_sortie
 * @param id_trackers
 * @param type_line
 * @param freq
 * @param old_clusters
 * @param wkf
 * @param we
 * @param options
 */
template <typename... OPTS>
void feature_matching_hough_update_three_first(feature_matching_hough_ctx& ftx,
                                               std::vector<float>& frame1,
                                               std::vector<float>& frame2,
                                               image2d<uchar> &frame_grad,
                                               image2d<uchar> &frame_point,
                                               Sclare_rho scale,
                                               int type_video,image2d<uchar> img,
                                               int T_theta,int rhomax,bool first,
                                               Type_output type_sortie,int &id_trackers,
                                               Type_Lines type_line,Frequence freq,
                                               std::vector<vint2>& old_clusters,
                                               int wkf,With_Entries we,
                                               OPTS... options)
{
    ftx.frame_id++;

    // Options.
    auto opts = D(options...);

    const int slot_hough = opts.get(_slot_hough, 5);
    const int max_trajectory_length = opts.get(_max_trajectory_length, 15);
    const int m_first_lines = opts.get(_m_first_lines, 10);
    const int rayon_exclusion_theta = opts.get(_rayon_exclusion_theta, 15);
    const int rayon_exclusion_rho = opts.get(_rayon_exclusion_rho, 12);
    const float acc_threshold = opts.get(_acc_threshold, 100);
    const float mfwu = opts.get(_nombre_max_frame_without_update, 5);

    float max_of_accu = 0;
    std::vector<float> t_accumulator(rhomax*T_theta);
    std::fill(t_accumulator.begin(),t_accumulator.end(),0);
    std::list<vint2> kps;
    float grad_thresold = 0;


    std::vector<std::list<vint2>> line_all_points(rhomax*T_theta);
    std::vector<vint4> line_extremities;
    if(Type_Lines::ALL_POINTS==type_line)
    {
        //line_all_points.reserve(rhomax*T_theta);
    }
    else if(Type_Lines::EXTREMITE == type_line)
    {
        std::vector<vint4> temp_pt(rhomax*T_theta,vint4(-1,-1,1000,1000));
        line_extremities = temp_pt;
    }


    if(Type_output::VIDEO_HOUGH == type_sortie)
    {
        //kps = Hough_Lines_Parallel_Basic(img,t_accumulator,T_theta,max_of_accu);
        if(slot_hough==1)
        {
            kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,acc_threshold);
        }
        else
        {
            if(ftx.frame_id==0 || ftx.frame_id%slot_hough ==0)
            {
                //cout << "dense" << endl;
                kps = Hough_Lines_Parallel_Update_Threshold(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,grad_thresold,m_first_lines);
            }
            else
            {
                //cout << "sparse" << endl;
                kps = Hough_Lines_Parallel_Sparse(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,grad_thresold,m_first_lines);
            }
        }

        if(Type_output::VIDEO_HOUGH == type_sortie)
        {
            if(type_video==2)
            {
                auto frame3 = from_opencv<uchar>(accumulator_to_frame(t_accumulator,max_of_accu,rhomax,T_theta));
                vpp::copy(frame3,frame_point);
            }
            else if(type_video==1)
            {
                auto frame3 = from_opencv<uchar>(accumulator_to_frame(kps,rhomax,T_theta));
                vpp::copy(frame3,frame_point);
            }
        }
    }
    else if(Type_output::GRADIENT_VIDEO == type_sortie  || Type_output::ORIGINAL_VIDEO == type_sortie)
    {
        if(Type_Lines::ALL_POINTS==type_line)
        {

            kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,max_of_accu,frame_grad,line_all_points);
        }
        else if(Type_Lines::EXTREMITE == type_line)
        {
            kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,max_of_accu,frame_grad,line_extremities);
        }
        else if(Type_Lines::ONLY_POLAR==type_line)
        {
            if(Frequence::ALL_FRAME == freq)
            {
                kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,acc_threshold);
            }
            else if (Frequence::NOT_ALL == freq)
            {
                /*std::list<cv::LineIterator> list_interest;
                if(old_vects.size()>0)
                {
                    for(auto &o : old_vects )
                    {
                        vint4 ori = getLineFromPoint(o[0],o[1],T_theta,img.nrows(),img.ncols());
                        //cv::LineIterator line_iterate(img,cv::Point(ori[0],ori[1]), cv::Point(ori[2],ori[3]),8);
                    }
                }*/
                kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,acc_threshold);
            }
        }
    }
    frame2 = t_accumulator;

    std::vector<vint2> temp_old(kps.size());
    int i=0;
    for(auto& kp : kps)
    {
        temp_old[i] = kp;
        i++;
    }

    old_clusters = temp_old;




    std::vector<std::vector<vfloat3>> three_best_matches;
    int nb = 0;
    if(first)
    {
        for(auto &kg : kps)
        {
            if(nb>m_first_lines)
                return;
            int ind = kg[0]*T_theta + kg[1];
            if(Type_Lines::ALL_POINTS==type_line)
            {
                ftx.list_track.push_back(track(kg,line_all_points[ind],id_trackers++,
                                               max_trajectory_length,t_accumulator[kg[0]*T_theta+kg[1]],
                        ftx.frame_id, motion_threshold));
            }
            else if(Type_Lines::EXTREMITE == type_line)
            {
                ftx.list_track.push_back(track(kg,line_extremities[ind],id_trackers++,
                                               max_trajectory_length,t_accumulator[kg[0]*T_theta+kg[1]],
                        ftx.frame_id, motion_threshold));
            }
            else if(Type_Lines::ONLY_POLAR==type_line)
            {
                ftx.list_track.push_back(track(kg,id_trackers++,
                                               max_trajectory_length,t_accumulator[kg[0]*T_theta+kg[1]],
                        ftx.frame_id,motion_threshold,wkf));
            }
            //cout << "first " << kg[0] << "  " << kg[1] << "  " << t_accumulator[ind] << endl;
            nb++;
        }
        frame1 = frame2;
        return;
    }

    timer t;
    t.start();


    std::vector<vint2> new_values;
    std::vector<int> up_new_values;
    for(auto &kg : kps)
    {
        new_values.push_back(kg);
        up_new_values.push_back(0);
    }

    if(new_values.size()==0)
    {
        frame1 = frame2;
        return;
    }


    for(int i = 0 ; i < ftx.list_track.size() ; i++ )
    {
        //cout << " i = " << i << endl;
        std::vector<vfloat3> mins(3,vfloat3(0,0,-1));
        int maj = 0;
        float theta_p = ((2*M_PI*(ftx.list_track[i].last_point)[1])/ (T_theta-1)) - M_PI;
        float rho_p = (ftx.list_track[i].last_point)[0];
        //cout << "point " << p_tt[0] << "  " << p_tt[1] << endl;
        for(int j = 0 ; j < new_values.size(); j++ )
        {
            if(j>i-10 && j<i+10)
            {
                /*vint2 p_t = ftx.list_track[i].last_point;
                vint2 p_r = new_values[j];
                //int ind1 = p_t[0]*T_theta + p_t[1];
                //int ind2 = p_r[0]*T_theta + p_r[1];
                /*if(fabs(p_t[1]-p_r[1]) > T_theta-50)
                {
                    p_r[1] = T_theta - p_r[1];
                }*/
                float theta_q = ((2*M_PI*(new_values[j])[1])/ (T_theta-1)) - M_PI;
                float rho_q = (new_values[j])[0];
                double the_norm = sqrt(rhomax*rhomax*pow(sin(theta_p-theta_q),2) + pow(rho_p-rho_q,2));
                //double the_nm = (ftx.list_track[i].last_point - new_values[j]).norm();

                //cout << the_norm <<  endl;
                if(the_norm<motion_threshold)
                {

                    /*cout << "Point " << (ftx.list_track[i].last_point)[0] << "  "
                         << (ftx.list_track[i].last_point)[1] << " , " << (new_values[j])[0] <<
                            "  " << (new_values[j])[1] << " norme " << the_norm <<  endl;*/
                    up_new_values[j] = -1;
                    if((mins[0])[2]==-1 || the_norm<(mins[0])[2])
                    {
                        mins[2] = mins[1];
                        mins[1] = mins[0];
                        mins[0] = vfloat3(j,i,the_norm);
                        maj = 1;
                    }
                    else if((mins[1])[2]==-1 || the_norm<(mins[1])[2])
                    {
                        mins[2] = mins[1];
                        mins[1] = vfloat3(j,i,the_norm);
                        maj = 1;
                    }
                    else if((mins[2])[2]==-1 || the_norm<(mins[2])[2])
                    {
                        mins[2] = vfloat3(j,i,the_norm);
                        maj = 1;
                    }
                }
            }
        }
        //cout << "\n\n" << endl;
        if(maj==0)
        {
            /*if(ftx.frame_id>=460)
            {
                vint2 p1 = ftx.list_track[i].last_point;
                //int ind = p1[0]*T_theta + p1[1];
                //cout << "Perdugg : " << p1[0] << " , " << p1[1] << endl;
            }*/
            ftx.list_track[i].die();
            ftx.list_track[i].frame_without_update++;
        }
        three_best_matches.push_back(mins);
    }


    //cout << " size of three_best_matches " << three_best_matches.size() << endl;

    int taille_theta = 40;//30
    int taille_rho = 40;//24
    int mid_theta = 20;
    int mid_rho = 20;
    //cout << "mid " << mid << endl;
    int q = 0;
    std::vector<int> updated_trackers(ftx.list_track.size(),-1);

    if(three_best_matches.size()==0)
    {
        frame1 = frame2;
        return;
    }
    for(auto &three_max : three_best_matches)
    {

        std::vector<vfloat3> val = three_max;
        vfloat3 diff_mat = vfloat3(-1,-1,-1);
        int maj_ind = 0;
        float d1 = 0;
        float d2 = 0;
        if((val[1])[2]!=-1)
        {
            d1 = (val[0])[2] / (val[1])[2];
        }
        if(d1 > 0)
        {
            d2 = (val[1])[2] / (val[2])[2];
            int taille = val.size();
            if(d2 > 0.4 )
                taille--;
            for(int i = 0; i < taille ; i++)
            {
                std::vector<float> local_area1(taille_theta*taille_rho,0);
                std::vector<float> local_area2(taille_theta*taille_rho,0);
                if( (val[i])[2] != -1  )
                {
                    maj_ind = 1;
                    vint2 p2 = new_values[(val[i])[0]];
                    vint2 p1 = ftx.list_track[(val[i])[1]].last_point;
                    int a = 0;
                    for(int r = p1[0]-mid_rho ; r < p1[0]+mid_rho ;r++ ,a++)
                    {
                        int b =0;
                        for(int t = p1[1]-mid_theta ; t < p1[1]+mid_theta ; t++ ,b++)
                        {
                            if(r>=rhomax || r<0 || t>=T_theta || t<0)
                            {
                                local_area1[a*taille_theta + b] = 0;
                            }
                            else
                                local_area1[a*taille_theta + b] = frame1[r*T_theta + t];
                        }
                    }
                    a = 0;
                    for(int r = p2[0]-mid_rho ; r < p2[0]+mid_rho ;r++ , a++)
                    {
                        int b = 0;
                        for(int t = p2[1]-mid_theta ; t < p2[1]+mid_theta ; t++, b++)
                        {
                            if(r>=rhomax || r<0 || t>=T_theta || t<0)
                            {
                                local_area2[a*taille_theta + b] = 0;
                            }
                            else
                                local_area2[a*taille_theta + b] = frame2[r*T_theta + t];
                        }
                    }
                    //diff_mat[i] = Distance_between_curve_L2(local_area1,local_area2,taille_theta,taille_rho);
                    diff_mat[i] = pearsoncoeff(local_area1,local_area2);
                    //cout << "difference " << diff_mat[i] << endl;
                    //cout << "p1 = [" << p1[0] << " , " << p1[1]  << "]  p2 = [ " << p2[0] << " , " << p2[1] << " ]    distance  = "
                    //      << diff_mat[i] << " et la norme " << (val[i])[2] << " corellation " << pearsoncoeff(local_area1,local_area2) << endl;
                }
            }
            //cout << endl << endl ;
        }
        else
        {
            maj_ind = 1;
            diff_mat[0] = 0;
        }


        if(maj_ind!=0)
        {
            float ind_val = diff_mat[0];
            int ind_min = -1;
            if(ind_val!=-1)
            {
                ind_min = 0;
                if(ind_val<diff_mat[1] && diff_mat[1]!=-1)
                {
                    ind_min = 1;
                }
                else if(ind_val<diff_mat[2] && diff_mat[2]!=-1)
                {
                    ind_min = 2;
                }
                ind_min = 0;

                vint2 p2 = new_values[(val[ind_min])[0]];
                //cout << "perhaps " << (val[ind_min])[0] << endl;
                //cout << "taille updated " << updated_trackers.size() << endl;
                //updated_trackers.push_back((val[ind_min])[0]);
                updated_trackers[q]= (val[ind_min])[0];
                vint2 p1 = ftx.list_track[(val[ind_min])[1]].last_point;
                int ind = p2[0]*T_theta + p2[1];

                /*cout << "p1 = [" << p1[0] << " , " << p1[1]  << "]  p2 = [ " << p2[0] << " , " << p2[1] <<
                        "]  val " << t_accumulator[p2[0]*T_theta+p2[1]] << endl;*/

                if(Type_Lines::ALL_POINTS==type_line)
                {
                    ftx.list_track[(val[ind_min])[1]].add_new_point(p2,line_all_points[ind],
                                                                  t_accumulator[p2[0]*T_theta+p2[1]],ftx.frame_id);
                }
                else if(Type_Lines::EXTREMITE == type_line)
                {
                    ftx.list_track[(val[ind_min])[1]].add_new_point(p2,line_extremities[ind],
                                                                  t_accumulator[p2[0]*T_theta+p2[1]],ftx.frame_id);
                }
                else if(Type_Lines::ONLY_POLAR==type_line)
                {
                    ftx.list_track[(val[ind_min])[1]].add_new_point(p2,
                                                                  t_accumulator[p2[0]*T_theta+p2[1]],ftx.frame_id);
                }
            }
        }
        //cout << "i eme point " << q++ << endl;
    }



    // cout << "\n\n\n" << endl;

    for(int i = 0 ; i < ftx.list_track.size() ; i++)
    {
        if(wkf)
        {
            if(!ftx.list_track[i].isAlive())
            {
                if(ftx.list_track[i].frame_without_update > mfwu)
                {
                    ftx.list_track.erase(ftx.list_track.begin()+i+1);
                }
                else
                {
                    ftx.list_track[i].resurrect();
                    ftx.list_track[i].only_update_trajectory();
                }
            }
        }
        else
        {
            if(!ftx.list_track[i].isAlive())
            {
                ftx.list_track.erase(ftx.list_track.begin()+i+1);
            }
        }
    }

    if(With_Entries::YES == we)
    {
        for(int i = 0 ; i < new_values.size() ; i++)
        {
            if(std::find(updated_trackers.begin(), updated_trackers.end(), i) != updated_trackers.end())
            {
                ///
                //cout << "ind " << i << endl;
            }
            else
            {
                if(up_new_values[i]==0 && i<m_first_lines)
                {
                    vint2 p2 = new_values[i];
                    int ind = p2[0]*T_theta + p2[1];
                    //cout << "valeur " << frame2[ind] << endl;
                    if(frame2[ind]>grad_thresold)
                    {
                        if(Type_Lines::ALL_POINTS == type_line)
                        {
                            ftx.list_track.push_back(track(p2,line_all_points[ind],id_trackers++,max_trajectory_length,
                                                           t_accumulator[p2[0]*T_theta+p2[1]],ftx.frame_id, motion_threshold));

                        }
                        else if(Type_Lines::EXTREMITE == type_line)
                        {
                            ftx.list_track.push_back(track(p2,line_extremities[ind],id_trackers++,max_trajectory_length,
                                                           t_accumulator[p2[0]*T_theta+p2[1]],ftx.frame_id, motion_threshold));
                        }
                        else if(Type_Lines::ONLY_POLAR == type_line)
                        {
                            ftx.list_track.push_back(track(p2,id_trackers++,max_trajectory_length,
                                                           t_accumulator[p2[0]*T_theta+p2[1]],ftx.frame_id,
                                    motion_threshold,wkf));
                        }
                    }
                }
            }
        }
    }

    frame1 = frame2;

    std::sort(ftx.list_track.begin(), ftx.list_track.end(), higher_gradient());

}



/**
 * @brief feature_matching_hough_update_N_first
 * @param ftx
 * @param frame1
 * @param frame2
 * @param list_clusters1
 * @param list_clusters2
 * @param frame_grad
 * @param frame_point
 * @param scale
 * @param type_video
 * @param img
 * @param T_theta
 * @param rhomax
 * @param first
 * @param type_sortie
 * @param id_trackers
 * @param type_line
 * @param freq
 * @param old_clusters
 * @param wkf
 * @param N
 * @param modulo_cp
 * @param options
 */
template <typename... OPTS>
void feature_matching_hough_update_N_first(feature_matching_hough_ctx& ftx,
                                           std::vector<float>& frame1,
                                           std::vector<float>& frame2,
                                           std::vector<vint2>& list_clusters1,
                                           std::vector<vint2>& list_clusters2,
                                           image2d<uchar> &frame_grad,
                                           image2d<uchar> &frame_point,
                                           Sclare_rho scale,
                                           int type_video,image2d<uchar> img,
                                           int T_theta,int rhomax,bool first,
                                           Type_output type_sortie,int &id_trackers,
                                           Type_Lines type_line,Frequence freq,
                                           std::vector<vint2>& old_clusters,
                                           int wkf,int N,int modulo_cp,
                                           OPTS... options)
{
    ftx.frame_id++;

    // Options.
    auto opts = D(options...);

    const int slot_hough = opts.get(_slot_hough, 5);
    const int max_trajectory_length = opts.get(_max_trajectory_length, 15);
    const int m_first_lines = opts.get(_m_first_lines, 10);
    const int rayon_exclusion_theta = opts.get(_rayon_exclusion_theta, 15);
    const int rayon_exclusion_rho = opts.get(_rayon_exclusion_rho, 12);
    const float acc_threshold = opts.get(_acc_threshold, 100);
    const float mfwu = opts.get(_nombre_max_frame_without_update, 5);

    float max_of_accu = 0;
    std::vector<float> t_accumulator(rhomax*T_theta);
    std::fill(t_accumulator.begin(),t_accumulator.end(),0);
    std::list<vint2> kps;
    float grad_thresold = 0;
    std::vector<std::list<vint2>> line_all_points(rhomax*T_theta);
    std::vector<vint4> line_extremities;


    if(Type_Lines::ALL_POINTS==type_line)
    {
        //line_all_points.reserve(rhomax*T_theta);
    }
    else if(Type_Lines::EXTREMITE == type_line)
    {
        std::vector<vint4> temp_pt(rhomax*T_theta,vint4(-1,-1,1000,1000));
        line_extremities = temp_pt;
    }

    if(Type_output::VIDEO_HOUGH == type_sortie)
    {
        //kps = Hough_Lines_Parallel_Basic(img,t_accumulator,T_theta,max_of_accu);
        if(slot_hough==1)
        {
            kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,acc_threshold);
        }
        else
        {
            if(ftx.frame_id==0 || ftx.frame_id%slot_hough ==0)
            {
                //cout << "dense" << endl;
                kps = Hough_Lines_Parallel_Update_Threshold(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,grad_thresold,m_first_lines);
            }
            else
            {
                //cout << "sparse" << endl;
                kps = Hough_Lines_Parallel_Sparse(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,grad_thresold,m_first_lines);
            }
        }

        if(Type_output::VIDEO_HOUGH == type_sortie)
        {
            if(type_video==2)
            {
                auto frame3 = from_opencv<uchar>(accumulator_to_frame(t_accumulator,max_of_accu,rhomax,T_theta));
                vpp::copy(frame3,frame_point);
            }
            else if(type_video==1)
            {
                auto frame3 = from_opencv<uchar>(accumulator_to_frame(kps,rhomax,T_theta));
                vpp::copy(frame3,frame_point);
            }
        }
    }
    else if(Type_output::GRADIENT_VIDEO == type_sortie  || Type_output::ORIGINAL_VIDEO == type_sortie)
    {
        if(Type_Lines::ALL_POINTS==type_line)
        {

            kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,max_of_accu,frame_grad,line_all_points);
        }
        else if(Type_Lines::EXTREMITE == type_line)
        {
            kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,max_of_accu,frame_grad,line_extremities);
        }
        else if(Type_Lines::ONLY_POLAR==type_line)
        {
            if(Frequence::ALL_FRAME == freq)
            {
                kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,acc_threshold);
            }
            else if (Frequence::NOT_ALL == freq)
            {
                kps = Hough_Lines_Parallel(img,t_accumulator,T_theta,rhomax,max_of_accu,frame_grad,acc_threshold);
            }
        }
    }
    if(ftx.frame_id%2==0)
    {
        frame1 = t_accumulator;
        for(auto& kp : kps)
        {
            list_clusters1.push_back(kp);
        }
    }
    else
    {
        frame2 = t_accumulator;
        for(auto& kp : kps)
        {
            list_clusters2.push_back(kp);
        }
    }


    int nb = 0;
    int alpha = 10;
    if(first)
    {
        for(auto &kg : kps)
        {
            if(nb>m_first_lines)
                break;
            int ind = kg[0]*T_theta + kg[1];
            if(Type_Lines::ALL_POINTS==type_line)
            {
                ftx.list_track.push_back(track(kg,line_all_points[ind],id_trackers++,
                                               max_trajectory_length,t_accumulator[kg[0]*T_theta+kg[1]],
                        ftx.frame_id, motion_threshold));
            }
            else if(Type_Lines::EXTREMITE == type_line)
            {
                ftx.list_track.push_back(track(kg,line_extremities[ind],id_trackers++,
                                               max_trajectory_length,t_accumulator[kg[0]*T_theta+kg[1]],
                        ftx.frame_id, motion_threshold));
            }
            else if(Type_Lines::ONLY_POLAR==type_line)
            {
                ftx.list_track.push_back(track(kg,id_trackers++,
                                               max_trajectory_length,t_accumulator[kg[0]*T_theta+kg[1]],
                        ftx.frame_id,motion_threshold,wkf));
            }
            nb++;
        }
        return;
    }


    /*
    std::list<std::vector<float>> kps0;
    nb = 0;
    if(ftx.frame_id==1)
    {
        for(int i = 0 ; i < ftx.list_track.size() ; i++ )
        {
            std::vector<float> dat(3,0);
            dat[0] = (ftx.list_track[i].last_point)[0];
            dat[1] = (ftx.list_track[i].last_point)[1];
            dat[2] = i;
            kps0.push_back(dat);
        }
        Kd_tree kdt(ftx.frame_id);
        kdt.createTree(kps0);
    }
    else
    {
        if(ftx.frame_id%2==0)
        {
            for(int i = 0 ; i < list_clusters1.size() ; i++)
            {
                if(i< list_clusters1.size() + alpha)
                {

                }
            }
        }
    }*/

}


/**
 * @brief compute_distance_hough_space
 * @param rhomax
 * @param rho_p
 * @param theta_p
 * @param rho_q
 * @param theta_q
 * @return
 */
inline
float compute_distance_hough_space(float rhomax,float rho_p,float theta_p,float rho_q,float theta_q)
{
    return sqrt(rhomax*rhomax*pow(sin(theta_p-theta_q),2) + pow(rho_p-rho_q,2));
}


/**
 * @brief copy_vector
 * @param src
 * @param dest
 */
inline
void copy_vector(std::vector<float> src,std::vector<float> &dest)
{
    if(src.size() ==0 || src.size()!=dest.size())
    {
        return;
    }
    else
    {
#pragma omp parallel for
        for(int i=0 ; i < src.size() ; i++)
        {
            dest[i] = src[i];
        }
    }
}


/**
 * @brief get_duplicata
 * @param ftx
 * @return
 */
std::vector<int> get_duplicata(feature_matching_hough_ctx ftx)
{
    std::vector<int> duplics;
    for(int i = 0 ; i < ftx.list_track.size()-1  ; i++ )
    {
        for(int j = i+1; j < ftx.list_track.size() ; j++)
        {
            if(ftx.list_track[i].gradients ==  ftx.list_track[j].gradients
                    && (ftx.list_track[i].last_point - ftx.list_track[j].last_point).norm()<2 )
            {
                if(std::find(duplics.begin(), duplics.end(), j) == duplics.end())
                {
                    duplics.push_back(j);
                }
            }
        }
    }
    return duplics;
}


/**
 * @brief distance_between_curve_l1
 * @param frame1
 * @param frame2
 * @param taille
 * @return
 */
float distance_between_curve_l1(std::vector<float>  frame1,std::vector<float>  frame2, int taille)
{
    float diff = 0;
    for(int i = 0 ; i < taille ; i++)
    {
        for(int j =0 ; j < taille ; j++)
        {
            diff += fabs(frame1[i * taille + j] - frame2[i * taille + j]);
        }
    }
    return diff;
}


/**
 * @brief distance_between_curve_l2
 * @param frame1
 * @param frame2
 * @param taille_theta
 * @param taille_rho
 * @return
 */
float distance_between_curve_l2(std::vector<float>  frame1, std::vector<float>  frame2, int taille_theta, int taille_rho)
{
    float diff = 0;
    for(int rho = 0 ; rho < taille_rho ; rho++)
    {
        for(int theta =0 ; theta < taille_theta ; theta++)
        {
            diff += pow((frame1[rho * taille_theta + theta] - frame2[rho * taille_theta + theta]),2);
            //cout << "diff " << diff << endl;
        }
    }
    diff = sqrt(diff);
    return diff;
}

/**
 * @brief correlation_matrix_pearson
 * @param M1
 * @param M2
 * @param taille
 * @return
 */
float correlation_matrix_pearson(Eigen::MatrixXd M1,Eigen::MatrixXd M2,int taille)
{

}


/**
 * @brief sum_vector
 * @param a
 * @return
 */
float sum_vector(std::vector<float> a)
{
    float s = 0;
    for (int i = 0; i < a.size(); i++)
    {
        s += a[i];
    }
    return s;
}


/**
 * @brief mean_vector
 * @param a
 * @return
 */
float mean_vector(std::vector<float> a)
{
    return sum_vector(a) / a.size();
}


/**
 * @brief sqsum
 * @param a
 * @return
 */
float sqsum(std::vector<float> a)
{
    float s = 0;
    for (int i = 0; i < a.size(); i++)
    {
        s += pow(a[i], 2);
    }
    return s;
}


/**
 * @brief stdev
 * @param nums
 * @return
 */
float stdev(std::vector<float> nums)
{
    float N = nums.size();
    return pow(sqsum(nums) / N - pow(sum_vector(nums) / N, 2), 0.5);
}


/**
 * @brief operator -
 * @param a
 * @param b
 * @return
 */
std::vector<float> operator-(std::vector<float> a, float b)
{
    std::vector<float> retvect;
    for (int i = 0; i < a.size(); i++)
    {
        retvect.push_back(a[i] - b);
    }
    return retvect;
}


/**
 * @brief operator *
 * @param a
 * @param b
 * @return
 */
std::vector<float> operator*(std::vector<float> a, std::vector<float> b)
{
    std::vector<float> retvect;
    for (int i = 0; i < a.size() ; i++)
    {
        retvect.push_back(a[i] * b[i]);
    }
    return retvect;
}


/**
 * @brief pearsoncoeff
 * @param X
 * @param Y
 * @return
 */
float pearsoncoeff(std::vector<float> X, std::vector<float> Y)
{
    return sum_vector((X - mean_vector(X))*(Y - mean_vector(Y))) / (X.size()*stdev(X)* stdev(Y));
}


/**
 * @brief brute_force_matching_basic_parallel
 * @param descriptor1
 * @param descriptor2
 * @param matches
 * @param type
 */
void brute_force_matching_basic_parallel(std::vector<vint2> descriptor1, std::vector<vint2> descriptor2,std::vector<int>& matches, int type)
{
    int size1 = descriptor1.size();
    int size2 = descriptor2.size();
    auto domain = make_box2d(size1,size2);
    std::vector<int> min_dist(size1,-1);
    image2d<float> mat_distance(domain);
    image2d<float> A(domain);
    pixel_wise(mat_distance, mat_distance.domain()) | [&] (auto &m, vint2 coord) {
        m = 0;
        int c2 = coord[1];
        int c1 = coord[0];
        float min_ = (descriptor1[c1] - descriptor2[c2]).norm();
        if(min_dist[c1]==-1 || min_dist[c1]<min_)
        {
#pragma omp critical
            {
                min_dist[c1] = min_;
                matches[c1] = c2;
            }
        }
    };

}


/**
 * @brief brute_force_matching_basic
 * @param descriptor1
 * @param descriptor2
 * @param matches
 * @param type
 */
void brute_force_matching_basic(std::vector<vint2> descriptor1, std::vector<vint2> descriptor2,std::vector<int>& matches, int type)
{
    int size1 = descriptor1.size();
    int size2 = descriptor2.size();
    auto domain = make_box2d(size1,size2);
    std::vector<int> min_dist(size1,-1);
    image2d<float> mat_distance(domain);
    image2d<float> A(domain);
    pixel_wise(mat_distance, mat_distance.domain()) | [&] (auto &m, vint2 coord) {
        m = 0;
        int c2 = coord[1];
        int c1 = coord[0];
        float min_ = (descriptor1[c1] - descriptor2[c2]).norm();
        if(min_dist[c1]==-1 || min_dist[c1]<min_)
        {
#pragma omp critical
            {
                min_dist[c1] = min_;
                matches[c1] = c2;
            }
        }
    };

}

}

