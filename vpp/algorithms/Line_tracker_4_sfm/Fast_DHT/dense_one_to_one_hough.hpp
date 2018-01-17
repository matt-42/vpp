


#include "dense_one_to_one_hough.hh"

namespace vpp{

/**
 * @brief Hough_Lines_Parallel_Kmeans
 * @param img
 * @param t_accumulator
 * @param Theta_max
 * @param rhomax
 * @param max_of_the_accu
 * @param grad_img
 * @return
 */
std::list<vint2> Hough_Lines_Parallel_Kmeans(image2d<uchar> img,
                                             std::vector<float>& t_accumulator,
                                             int Theta_max,int rhomax, float& max_of_the_accu
                                             ,  image2d<uchar> &grad_img )
{
    //timer t;
    //t.start();
    //image2d<vuchar1> img(imag.domain());
    int ncols = img.ncols();
    int nrows = img.nrows();
    float T_theta = Theta_max;
    std::list<vint2> interestedPoints;
    int rho_T = int(sqrt(pow(ncols,2)+pow(nrows,2)));
    int scale = (float)rho_T/(float)rhomax;
    //cout << "here " << endl;
    //std::vector<int> nombre_vote(rhomax*T_theta,0);

    pixel_wise(grad_img, relative_access(img), img.domain()) | [&] (auto& g, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>3 && y>3 && x<ncols-3 && y<nrows-3)
        {
            float dx = 0;
            float dy = 0;
            dx = -(i(1,-1))  +  (i(1,1))
                    -2* (i(0,-1))  +  2*(i(0,1))
                    - (i(-1,-1))  + (i(-1,1)) ;
            dy = (i(1,-1))  + 2*(i(1,0)) + (i(1,1))
                    - (i(-1,-1))  -2* (i(-1,0))  - (i(-1,1)) ;
            dx /= 4;
            dy /=4;
            float deltaI = sqrt( dx*dx + dy*dy);
            //timer t;
            //t.start();
            if(deltaI>0)
            {
                float d = x*dx + y*dy;
                float rho = fabs(d/deltaI);
                int index_rho = (int)trunc(rho);
                float poids_rho = 1 - rho + index_rho;
                float theta;
                float alpha;
                float dist_sign = x*dx + y * dy;

                // Calcul de l'argument du gradient
                if (dx)
                    alpha = atan(dy/dx);
                else
                    alpha = M_PI/2;
                // Calcul de theta, l'angle entre Ox et la droite
                // passant par O et perpendiculaire à la droite D
                if ((alpha < 0)&&(dy*dist_sign > 0))
                    theta = M_PI + alpha;
                else
                    theta = alpha;

                //Prendre les coordonnees
                float pos_theta = ((theta + M_PI)*(T_theta-1))/(2*M_PI);
                int index_theta = (int)(trunc(pos_theta));
                {
                    float poids_theta =  1 - pos_theta + index_theta;
                    float vote_total = deltaI;
                    //#pragma omp critical
                    {
                        t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                        int ind = index_rho*T_theta + index_theta;
                        //nombre_vote[ind]++;
                        if (poids_rho < 1)
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                            ind = (index_rho+1)*T_theta + index_theta;
                            //nombre_vote[ind]++;
                        }
                        if (poids_theta < 1)
                        {
                            t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                            ind = index_rho*T_theta + index_theta+1;
                            //nombre_vote[ind]++;
                        }
                        if ((poids_rho < 1)&&(poids_theta<1))
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                            ind = (index_rho+1)*T_theta + index_theta+1;
                            //nombre_vote[ind]++;
                        }
                    }
                }
            }
            //           t.end();
            //#pragma omp atomic
            //          execution_time += t.us();
            g = uchar(round(deltaI));
        }
        else
            g = 0;
    };


    //t.end();
    //cout << "hough timer " << t.us() << endl;


    //cv::imwrite("sortieP.jpg", to_opencv(grad_img));
    Mat cimg = to_opencv(grad_img);



    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);


    //#pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;


#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>50)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }

            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;

    std::list<vfloat3> list_temp;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = temp_acc[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( temp_acc[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = temp_acc[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == temp_acc[rho*T_theta + theta] && max > 100)
            {
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
                //cout << "max " << t_accumulator[rho*T_theta + theta] << " et rho " << rho << " et theta " << theta <<  endl;
            }
        }
    }


    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});

    Mat result;
    cvtColor(cimg,result,CV_GRAY2BGR);

    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        if(nb_lines>300)
            break;
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPoints  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<12 && fabs(val[1]-coord[1])<15)
            {
                found = 1;
                break;
            }
        }
        if(found==0)
        {
            interestedPoints.push_back(coord);
            int ind = coord[0]*T_theta + coord[1];
            //cout << "max aa " << t_accumulator[ind] << " et rho " << coord[0] << " et theta " << coord[1] <<  endl;

            nb_lines++;
        }
    }

    //cout << "nombre " << nb_lines << endl;
    //cv::imwrite("okay.bmp", result);

    return interestedPoints;
}


/**
 * @brief Hough_Lines_Parallel
 * @param img
 * @param t_accumulator
 * @param Theta_max
 * @param rhomax
 * @param max_of_the_accu
 * @param grad_img
 * @param acc_threshold
 * @return
 */
std::list<vint2> Hough_Lines_Parallel(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max,int rhomax, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img,float acc_threshold )
{
    int ncols = img.ncols();
    int nrows = img.nrows();
    float T_theta = Theta_max;
    std::list<vint2> interestedPoints;

    pixel_wise(grad_img, relative_access(img), img.domain()) | [&] (auto& g, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>3 && y>3 && x<ncols-3 && y<nrows-3)
        {
            float dx = 0;
            float dy = 0;
            dx = -(i(1,-1))  +  (i(1,1))
                    -2* (i(0,-1))  +  2*(i(0,1))
                    - (i(-1,-1))  + (i(-1,1)) ;
            dy = (i(1,-1))  + 2*(i(1,0)) + (i(1,1))
                    - (i(-1,-1))  -2* (i(-1,0))  - (i(-1,1)) ;
            dx /= 4;
            dy /=4;
            float deltaI = sqrt( dx*dx + dy*dy);
            if(deltaI>0)
            {
                float dist_sign = x*dx + y*dy;
                float rho = fabs(dist_sign/deltaI);
                int index_rho = (int)trunc(rho);
                float poids_rho = 1 - rho + index_rho;
                float theta;
                float alpha;

                /*if (dx)
                {
                    alpha = atan(dx/dy);
                }
                else
                {
                    alpha = M_PI_2;
                }
                if ((alpha < 0)&&(dy*dist_sign > 0))
                    theta = M_PI + alpha;
                else
                    theta = alpha;*/


                if(dx*dy<0 && dist_sign*dy>0)
                    theta = M_PI + atan(dy/dx);
                else
                    theta = atan(dy/dx);


                //Prendre les coordonnees
                float pos_theta = ((theta + M_PI)*(T_theta-1))/(2*M_PI);
                int index_theta = (int)(trunc(pos_theta));

                {
                    float poids_theta =  1 - pos_theta + index_theta;
                    float vote_total = deltaI;
                    //#pragma omp critical
                    {
                        t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                        if (poids_rho < 1)
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                        }
                        if (poids_theta < 1)
                        {
                            t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                        }
                        if ((poids_rho < 1)&&(poids_theta<1))
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                        }
                    }
                }
            }
            g = uchar(round(deltaI));
        }
        else
        {
            g = 0;
        }
    };

    //Mat cimg = to_opencv(img);

    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);


    #pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;


#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>50)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }
            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;

    std::list<vfloat3> list_temp;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = temp_acc[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( temp_acc[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = temp_acc[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == temp_acc[rho*T_theta + theta] && max > acc_threshold)
            {
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
            }
        }
    }


    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});

    /*Mat result;
    cvtColor(cimg,result,CV_GRAY2BGR);*/

    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        if(nb_lines>300)
            break;
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPoints  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<12 && fabs(val[1]-coord[1])<15)
            {
                found = 1;
                break;
            }
        }
        if(found==0)
        {
            interestedPoints.push_back(coord);
            /*float cosinus,sinus;
            vint4 ligne = getLineFromPoint_Fast(coord[0],coord[1],T_theta,nrows,ncols,cosinus,sinus);
            vint4 ligne2 = getLineFromPoint(coord[0],coord[1],T_theta,nrows,ncols);
            cv::line(result, cv::Point(ligne[0], ligne[1]), cv::Point(ligne[2], ligne[3]), Scalar(0,255,125),2);
            cout << "1 " << ligne << endl << endl;
            cout << "2 " << ligne2 << endl << endl;*/
            nb_lines++;
        }
    }
    //cv::imwrite("okay.bmp", result);
    return interestedPoints;
}


/**
 * @brief Hough_Lines_Parallel_Update_Threshold
 * @param img
 * @param t_accumulator
 * @param Theta_max
 * @param rhomax
 * @param max_of_the_accu
 * @param grad_img
 * @param grad_threshold
 * @param N
 * @return
 */
std::list<vint2> Hough_Lines_Parallel_Update_Threshold(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max,int rhomax, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img , float& grad_threshold , int N)
{
    int ncols = img.ncols();
    int nrows = img.nrows();
    float T_theta = Theta_max;
    std::list<vint2> interestedPoints;
    std::vector<int> nombre_vote(rhomax*T_theta,0);

    pixel_wise(grad_img, relative_access(img), img.domain()) | [&] (auto& g, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>1 && y>1 && x<ncols-1 && y<nrows-1)
        {
            float dx = 0;
            float dy = 0;
            dx = -(i(1,-1))  +  (i(1,1))
                    -2* (i(0,-1))  +  2*(i(0,1))
                    - (i(-1,-1))  + (i(-1,1)) ;
            dy = (i(1,-1))  + 2*(i(1,0)) + (i(1,1))
                    - (i(-1,-1))  -2* (i(-1,0))  - (i(-1,1)) ;
            dx /= 4;
            dy /=4;
            float deltaI = sqrt( dx*dx + dy*dy);
            if(deltaI>0)
            {
                float d = x*dx + y*dy;
                float rho = fabs(d/deltaI);
                int index_rho = (int)trunc(rho);
                float poids_rho = 1 - rho + index_rho;
                float theta;
                float alpha;
                float dist_sign = x*dx + y * dy;
                if (dx)
                {
                    alpha = atan(dx/dy);
                }
                else
                {
                    alpha = M_PI_2;
                    //alpha2 = M_PI_2;
                }
                if ((alpha < 0)&&(dy*dist_sign > 0))
                    theta = M_PI + alpha;
                else
                    theta = alpha;

                //Prendre les coordonnees
                float pos_theta = ((theta + M_PI)*(T_theta-1))/(Two_PI);
                int index_theta = (int)(trunc(pos_theta));

                {
                    float poids_theta =  1 - pos_theta + index_theta;
                    float vote_total = deltaI;
                    //#pragma omp critical
                    {
                        t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                        int ind = index_rho*T_theta + index_theta;
                        nombre_vote[ind]++;
                        if (poids_rho < 1)
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                            ind = (index_rho+1)*T_theta + index_theta;
                            nombre_vote[ind]++;
                        }
                        if (poids_theta < 1)
                        {
                            t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                            ind = index_rho*T_theta + index_theta+1;
                            nombre_vote[ind]++;
                        }
                        if ((poids_rho < 1)&&(poids_theta<1))
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                            ind = (index_rho+1)*T_theta + index_theta+1;
                            nombre_vote[ind]++;
                        }
                    }
                }
            }
            g = uchar(round(deltaI));
        }
        else
        {
            g = 0;
        }
    };

    Mat cimg = to_opencv(grad_img);

    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);


    //#pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;


#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>50)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }

            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;

    std::list<vfloat3> list_temp;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = temp_acc[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( temp_acc[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = temp_acc[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == temp_acc[rho*T_theta + theta] && max > 100)
            {
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
            }
        }
    }


    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});

    Mat result;
    cvtColor(cimg,result,CV_GRAY2BGR);

    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        if(nb_lines>10*N)
            break;
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPoints  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<12 && fabs(val[1]-coord[1])<15)
            {
                found = 1;
                break;
            }
        }
        if(found==0)
        {
            interestedPoints.push_back(coord);
            if(nb_lines==N)
            {
                int ind = coord[0]*T_theta + coord[1];
                grad_threshold = t_accumulator[ind]/nombre_vote[ind];
                //cout << "Seuil " << grad_threshold << endl;
            }
            //float cosinus,sinus;
            //vint4 ligne = getLineFromPoint_Fast(coord[0],coord[1],T_theta,nrows,ncols,cosinus,sinus);
            //vint4 ligne2 = getLineFromPoint(coord[0],coord[1],T_theta,nrows,ncols);
            //cout << "1 " << ligne << endl << endl;
            //cout << "2 " << ligne2 << endl << endl;
            nb_lines++;
        }
    }
    return interestedPoints;
}

/**
 * @brief Hough_Lines_Parallel_Sparse
 * @param img
 * @param t_accumulator
 * @param Theta_max
 * @param rhomax
 * @param max_of_the_accu
 * @param grad_img
 * @param grad_threshold
 * @param N
 * @return
 */
std::list<vint2> Hough_Lines_Parallel_Sparse(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max,int rhomax, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img , float grad_threshold , int N)
{
    int ncols = img.ncols();
    int nrows = img.nrows();
    float T_theta = Theta_max;
    std::list<vint2> interestedPoints;

    pixel_wise(grad_img, relative_access(img), img.domain()) | [&] (auto& g, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>1 && y>1 && x<ncols-1 && y<nrows-1)
        {
            float dx = 0;
            float dy = 0;
            dx = -(i(1,-1))  +  (i(1,1))
                    -2* (i(0,-1))  +  2*(i(0,1))
                    - (i(-1,-1))  + (i(-1,1)) ;
            dy = (i(1,-1))  + 2*(i(1,0)) + (i(1,1))
                    - (i(-1,-1))  -2* (i(-1,0))  - (i(-1,1)) ;
            dx /= 4;
            dy /=4;
            float deltaI = sqrt( dx*dx + dy*dy);
            if(deltaI>grad_threshold)
            {
                float d = x*dx + y*dy;
                float rho = fabs(d/deltaI);
                int index_rho = (int)trunc(rho);
                float poids_rho = 1 - rho + index_rho;
                float theta;
                float alpha;
                float dist_sign = x*dx + y * dy;
                if (dx)
                {
                    alpha = atan(dx/dy);
                }
                else
                {
                    alpha = M_PI_2;
                    //alpha2 = M_PI_2;
                }
                if ((alpha < 0)&&(dy*dist_sign > 0))
                    theta = M_PI + alpha;
                else
                    theta = alpha;


                //Prendre les coordonnees
                float pos_theta = ((theta + M_PI)*(T_theta-1))/(Two_PI);
                int index_theta = (int)(trunc(pos_theta));

                {
                    float poids_theta =  1 - pos_theta + index_theta;
                    float vote_total = deltaI;
                    //#pragma omp critical
                    {
                        t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                        if (poids_rho < 1)
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                        }
                        if (poids_theta < 1)
                        {
                            t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                        }
                        if ((poids_rho < 1)&&(poids_theta<1))
                        {
                            t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                        }
                    }
                }
            }
            g = uchar(round(deltaI));
        }
        else
        {
            g = 0;
        }
    };

    Mat cimg = to_opencv(grad_img);

    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);


    //#pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;


#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>50)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }

            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;

    std::list<vfloat3> list_temp;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = temp_acc[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( temp_acc[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = temp_acc[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == temp_acc[rho*T_theta + theta] && max > 100)
            {
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
            }
        }
    }


    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});

    Mat result;
    cvtColor(cimg,result,CV_GRAY2BGR);

    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        if(nb_lines>10*N)
            break;
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPoints  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<12 && fabs(val[1]-coord[1])<15)
            {
                found = 1;
                break;
            }
        }
        if(found==0)
        {
            interestedPoints.push_back(coord);
            //float cosinus,sinus;
            //vint4 ligne = getLineFromPoint_Fast(coord[0],coord[1],T_theta,nrows,ncols,cosinus,sinus);
            //vint4 ligne2 = getLineFromPoint(coord[0],coord[1],T_theta,nrows,ncols);
            //cout << "1 " << ligne << endl << endl;
            //cout << "2 " << ligne2 << endl << endl;
            nb_lines++;
        }
    }
    return interestedPoints;
}




/**
 * @brief Hough_Lines_Parallel
 * @param img
 * @param t_accumulator
 * @param Theta_max
 * @param max_of_the_accu
 * @param grad_img
 * @param points50
 * @return
 */
std::list<vint2> Hough_Lines_Parallel(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img, std::vector<std::list<vint2>>& points50)
{
    //timer t;
    //t.start();
    typedef vfloat3 F;
    typedef vuchar3 V;
    //image2d<vuchar1> img(imag.domain());
    int ncols = img.ncols();
    int nrows = img.nrows();

    int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
    float T_theta = Theta_max;
    std::list<vint2> interestedPointsTemp;
    std::list<vint2> interestedPoints;
    std::vector<int> nombre_vote(rhomax*T_theta,0);
    pixel_wise(grad_img, relative_access(img), img.domain()) | [&] (auto& g, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>3 && y>3 && x<ncols-3 && y<nrows-3)
        {
            float dx = 0;
            float dy = 0;
            dx = -(i(1,-1))  +  (i(1,1))
                    -2* (i(0,-1))  +  2*(i(0,1))
                    - (i(-1,-1))  + (i(-1,1)) ;
            dy = (i(1,-1))  + 2*(i(1,0))  + (i(1,1))
                    - (i(-1,-1))  -2* (i(-1,0))  - (i(-1,1));
            dx /= 4;
            dy /=4;
            float deltaI = sqrt( dx*dx + dy*dy);
            if(deltaI>0)
            {
                float d = x*dx + y*dy;
                float rho = fabs(d/deltaI);
                int index_rho = (int)trunc(rho);
                float poids_rho = 1 - rho + index_rho;
                float theta;
                float alpha;
                float dist_sign = x*dx + y * dy;
                // Calcul de l'argument du gradient
                if (dx)
                    alpha = atan(dy/dx);
                else
                    alpha = M_PI/2;
                // Calcul de theta, l'angle entre Ox et la droite
                // passant par O et perpendiculaire à la droite D
                if ((alpha < 0)&&(dy*dist_sign > 0))
                    theta = M_PI + alpha;
                else
                    theta = alpha;

                float pos_theta = ((theta + M_PI)*(T_theta-1))/(2*M_PI);
                //Prendre les coordonnees
                int index_theta = (int)(trunc(pos_theta));
                float poids_theta =  1 - pos_theta + index_theta;
                float vote_total = deltaI;
#pragma omp critical
                {
                    t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                    int ind = index_rho*T_theta + index_theta;
                    nombre_vote[ind]++;
                    (points50[ind]).push_back(vint2(y,x));
                    if (poids_rho < 1)
                    {
                        t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                        ind = (index_rho+1)*T_theta + index_theta;
                        nombre_vote[ind]++;
                        (points50[ind]).push_back(vint2(y,x));
                    }
                    if (poids_theta < 1)
                    {
                        t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                        ind = index_rho*T_theta + index_theta+1;
                        nombre_vote[ind]++;
                        (points50[ind]).push_back(vint2(y,x));
                    }
                    if ((poids_rho < 1)&&(poids_theta<1))
                    {
                        t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                        ind = (index_rho+1)*T_theta + index_theta+1;
                        nombre_vote[ind]++;
                        (points50[ind]).push_back(vint2(y,x));
                    }
                }
            }
            g = uchar(round(deltaI));
            //g = 0;
        }
        else
            g = 0;
    };




    cv::imwrite("sortieP.jpg", to_opencv(grad_img));
    Mat cimg = to_opencv(grad_img);
    Mat result;
    cvtColor(cimg,result,CV_GRAY2BGR);

    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);


    //#pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;


#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>50)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }

            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;

    std::list<vfloat3> list_temp;

    std::list<vint2> max_locaux;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = temp_acc[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( temp_acc[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = temp_acc[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == temp_acc[rho*T_theta + theta] && max > 100)
            {
                max_locaux.push_back(vint2(rho,theta));
                //cout << "max aa " << t_accumulator[rho*T_theta + theta] << " et theta " << theta << " et rho " << rho <<  endl;
            }
        }
    }


    //cout << "la taille " << max_locaux.size() << endl;

    int tt =  max_locaux.size() ;

    for(auto &val : max_locaux)
    {
        int rho = val[0];
        int theta = val[1];
        /*float moyenne_pondere = 0;
        float quotient = 0;
        for(int i = rho - 1 ; i <= rho+1 ; i++  )
        {
            for(int j = theta - 1; j <= theta+1 ; j++ )
            {
                if(i>0 && i <rhomax && j >0 && j < T_theta)
                {
                    int ind = i * T_theta + theta;
                    moyenne_pondere += temp_acc[ind]*nombre_vote[ind];
                    quotient += nombre_vote[ind];
                }
            }
        }
        t_accumulator[rho*T_theta+theta] = moyenne_pondere/quotient;*/
        //cout << "max " << t_accumulator[rho*T_theta + theta] << " et theta " << theta << " et rho " << rho <<  endl;
        list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
    }



    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});


    //image2d<vuchar3> res = graylevel_to_rgb<vuchar3>(grad_img);



    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPointsTemp  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<5 && fabs(val[1]-coord[1])<5)
            {
                found = 1;
                break;
            }

        }

        if(found==0)
        {
            interestedPointsTemp.push_back(coord);
            /*int ind = coord[0]*T_theta + coord[1];
            std::list<vint2> ligne = points50[ind];
            #pragma omp parallel
            for(auto &l : ligne )
            {
                //res(l) = vuchar3(0,255,0);
                result.at<cv::Vec3b>(l[0],l[1])[0]  = 0;
                result.at<cv::Vec3b>(l[0],l[1])[1]  = 255;
                result.at<cv::Vec3b>(l[0],l[1])[2]  = 0;
            }*/
            nb_lines++;
        }
    }

    int nb_max = std::max(int(0.5*interestedPointsTemp.size()),10);

    nb_lines = 0;

    for(auto &l : interestedPointsTemp)
    {
        interestedPoints.push_back(l);
        if(nb_lines>300)
            break;
        nb_lines++;
    }

    //t.end();

    //cout << "hough timer " << t.us() << endl;
    //cv::imwrite("okay.bmp", result);

    return interestedPoints;
}


/**
 * @brief Hough_Lines_Parallel
 * @param img
 * @param t_accumulator
 * @param Theta_max
 * @param max_of_the_accu
 * @param grad_img
 * @param extremites
 * @return
 */
std::list<vint2> Hough_Lines_Parallel(image2d<uchar> img,
                                      std::vector<float>& t_accumulator,
                                      int Theta_max, float& max_of_the_accu
                                      ,  image2d<uchar> &grad_img,
                                      std::vector<vint4>& extremites)
{
    timer t;
    t.start();
    typedef vfloat3 F;
    typedef vuchar3 V;
    //image2d<vuchar1> img(imag.domain());
    int ncols = img.ncols();
    int nrows = img.nrows();

    int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
    float T_theta = Theta_max;
    std::list<vint2> interestedPoints;
    std::vector<int> nombre_vote(rhomax*T_theta,0);

    pixel_wise(grad_img, relative_access(img), img.domain()) | [&] (auto& g, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>3 && y>3 && x<ncols-3 && y<nrows-3)
        {
            float dx = 0;
            float dy = 0;
            dx = -(i(1,-1))  +  (i(1,1))
                    -2* (i(0,-1))  +  2*(i(0,1))
                    - (i(-1,-1))  + (i(-1,1)) ;
            dy = (i(1,-1))  + 2*(i(1,0))  + (i(1,1))
                    - (i(-1,-1))  -2* (i(-1,0))  - (i(-1,1));
            dx /= 4;
            dy /=4;
            float deltaI = sqrt( dx*dx + dy*dy);
            if(deltaI>100)
            {
                float d = x*dx + y*dy;
                float rho = fabs(d/deltaI);
                int index_rho = (int)trunc(rho);
                float poids_rho = 1 - rho + index_rho;
                float theta;
                float alpha;
                float dist_sign = x*dx + y * dy;
                /*// Calcul de l'argument du gradient
                if (dx)
                    alpha = atan(dy/dx);
                else
                    alpha = M_PI/2;
                // Calcul de theta, l'angle entre Ox et la droite
                // passant par O et perpendiculaire à la droite D
                if ((alpha < 0)&&(dy*dist_sign > 0))
                    theta = M_PI + alpha;
                else
                    theta = alpha;*/
                if(dx*dy<0 && dist_sign*dy>0)
                    theta = M_PI + atan(dy/dx);
                else
                    theta = atan(dy/dx);
                float pos_theta = ((theta + M_PI)*(T_theta-1))/(2*M_PI);
                //Prendre les coordonnees
                int index_theta = (int)(trunc(pos_theta));
                float poids_theta =  1 - pos_theta + index_theta;
                float vote_total = deltaI;
#pragma omp critical
                {
                    t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                    int ind = index_rho*T_theta + index_theta;
                    nombre_vote[ind]++;
                    if((extremites[ind])[0]==-1 || (extremites[ind])[1]==-1)
                    {
                        (extremites[ind])[0] = x;
                        (extremites[ind])[1] = y;
                        (extremites[ind])[2] = x;
                        (extremites[ind])[3] = y;
                    }
                    else
                    {
                        if( (extremites[ind])[0] < x  )
                        {
                            (extremites[ind])[0] = x;
                            (extremites[ind])[1] = y;
                        }
                        if(  (extremites[ind])[2]>=x )
                        {
                            (extremites[ind])[2] = x;
                            (extremites[ind])[3] = y;
                        }
                        if((extremites[ind])[0] == (extremites[ind])[2])
                        {
                            if( (extremites[ind])[1] < y  )
                            {
                                (extremites[ind])[0] = x;
                                (extremites[ind])[1] = y;
                            }
                            if( (extremites[ind])[3]>y )
                            {
                                (extremites[ind])[2] = x;
                                (extremites[ind])[3] = y;
                            }
                        }
                    }

                    if (poids_rho < 1)
                    {
                        t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                        ind = (index_rho+1)*T_theta + index_theta;
                        nombre_vote[ind]++;
                        if((extremites[ind])[0]==-1 || (extremites[ind])[1]==-1)
                        {
                            (extremites[ind])[0] = x;
                            (extremites[ind])[1] = y;
                            (extremites[ind])[2] = x;
                            (extremites[ind])[3] = y;
                        }
                        else
                        {
                            if( (extremites[ind])[0] < x  )
                            {
                                (extremites[ind])[0] = x;
                                (extremites[ind])[1] = y;
                            }
                            if(  (extremites[ind])[2]>=x )
                            {
                                (extremites[ind])[2] = x;
                                (extremites[ind])[3] = y;
                            }
                            if((extremites[ind])[0] == (extremites[ind])[2])
                            {
                                if( (extremites[ind])[1] < y  )
                                {
                                    (extremites[ind])[0] = x;
                                    (extremites[ind])[1] = y;
                                }
                                if( (extremites[ind])[3]>y )
                                {
                                    (extremites[ind])[2] = x;
                                    (extremites[ind])[3] = y;
                                }
                            }
                        }
                    }
                    if (poids_theta < 1)
                    {
                        t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                        ind = index_rho*T_theta + index_theta+1;
                        nombre_vote[ind]++;
                        if((extremites[ind])[0]==-1 || (extremites[ind])[1]==-1)
                        {
                            (extremites[ind])[0] = x;
                            (extremites[ind])[1] = y;
                            (extremites[ind])[2] = x;
                            (extremites[ind])[3] = y;
                        }
                        else
                        {
                            if( (extremites[ind])[0] < x  )
                            {
                                (extremites[ind])[0] = x;
                                (extremites[ind])[1] = y;
                            }
                            if(  (extremites[ind])[2]>=x )
                            {
                                (extremites[ind])[2] = x;
                                (extremites[ind])[3] = y;
                            }
                            if((extremites[ind])[0] == (extremites[ind])[2])
                            {
                                if( (extremites[ind])[1] < y  )
                                {
                                    (extremites[ind])[0] = x;
                                    (extremites[ind])[1] = y;
                                }
                                if( (extremites[ind])[3]>y )
                                {
                                    (extremites[ind])[2] = x;
                                    (extremites[ind])[3] = y;
                                }
                            }
                        }
                    }
                    if ((poids_rho < 1)&&(poids_theta<1))
                    {
                        t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                        ind = (index_rho+1)*T_theta + index_theta+1;
                        nombre_vote[ind]++;
                        if((extremites[ind])[0]==-1 || (extremites[ind])[1]==-1)
                        {
                            (extremites[ind])[0] = x;
                            (extremites[ind])[1] = y;
                            (extremites[ind])[2] = x;
                            (extremites[ind])[3] = y;
                        }
                        else
                        {
                            if( (extremites[ind])[0] < x  )
                            {
                                (extremites[ind])[0] = x;
                                (extremites[ind])[1] = y;
                            }
                            if(  (extremites[ind])[2]>=x )
                            {
                                (extremites[ind])[2] = x;
                                (extremites[ind])[3] = y;
                            }
                            if((extremites[ind])[0] == (extremites[ind])[2])
                            {
                                if( (extremites[ind])[1] < y  )
                                {
                                    (extremites[ind])[0] = x;
                                    (extremites[ind])[1] = y;
                                }
                                if( (extremites[ind])[3]>y )
                                {
                                    (extremites[ind])[2] = x;
                                    (extremites[ind])[3] = y;
                                }
                            }
                        }
                    }
                }
            }
            g = uchar(round(deltaI));
        }
        else
            g = 0;
    };





    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);


    //#pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;


#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>50)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }

            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;


    //std::list<vint2> max_locaux;
    std::list<vfloat3> list_temp;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = t_accumulator[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( t_accumulator[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = t_accumulator[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == t_accumulator[rho*T_theta + theta] && max > 100)
            {
                //max_locaux.push_back(vint2(rho,theta));
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
            }
        }
    }



    /*for(auto &val : max_locaux)
    {
        int rho = val[0];
        int theta = val[1];
        float moyenne_pondere = 0;
        float quotient = 0;
        for(int i = rho - 1 ; i <= rho+1 ; i++  )
        {
            for(int j = theta - 1; j <= theta+1 ; j++ )
            {
                if(i>0 && i <rhomax && j >0 && j < T_theta)
                {
                    int ind = i * T_theta + theta;
                    moyenne_pondere += temp_acc[ind]*nombre_vote[ind];
                    quotient += nombre_vote[ind];
                }
            }
        }
        t_accumulator[rho*T_theta+theta] = moyenne_pondere/quotient;
        list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
    }*/

    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});

    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        if(nb_lines>300)
            break;
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPoints  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<12 && fabs(val[1]-coord[1])<15)
            {
                found = 1;
                break;
            }
        }

        if(found==0)
        {
            interestedPoints.push_back(coord);
            /*int ind = coord[0]*T_theta + coord[1];
            int x1,y1,x2,y2;
            vint4 ligne = extremites[ind];
            x1 = ligne[0];
            y1 = ligne[1];
            x2 = ligne[2];
            y2 = ligne[3];
            cv::line(result, cv::Point(x1, y1), cv::Point(x2, y2), (0,255,255),2);*/
            nb_lines++;
        }
    }
    t.end();

    //cout << "hough timer " << t.us() << endl;
    //cout << "nombre " << lines_drawn << endl;
    //cv::imwrite("okay.bmp", result);


    return interestedPoints;
}


/**
 * @brief Hough_Lines_Parallel_one
 * @param bv
 * @param t_accumulator
 * @param Theta_max
 * @param max_of_the_accu
 * @param grad_img
 * @return
 */
std::list<vint2> Hough_Lines_Parallel_one(Mat bv,
                                          std::vector<float>& t_accumulator,
                                          int Theta_max, float& max_of_the_accu
                                          ,  Mat &grad_img )
{


    int ncols = bv.cols;
    int nrows = bv.rows;
    int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
    float T_theta = Theta_max;
    std::list<vint2> interestedPoints;
    int temps = 0;

    for(int y = 0 ; y < nrows ; y++)
    {
#pragma omp parallel for
        for(int x = 0 ; x < ncols ; x++)
        {

            if(x>3 && y>3 && x<ncols-3 && y<nrows-3)
            {
                float dx = 0;
                float dy = 0;
                dx = -(bv.at<uchar>(y+1,x-1))  +  (bv.at<uchar>(y+1,x+1))
                        -2* (bv.at<uchar>(y,x-1))  +  2*(bv.at<uchar>(y,x+1))
                        - (bv.at<uchar>(y-1,x-1))  + (bv.at<uchar>(y-1,x+1)) ;
                dy = (bv.at<uchar>(y+1,x-1))  + 2*(bv.at<uchar>(y+1,x)) + (bv.at<uchar>(y+1,x+1))
                        - (bv.at<uchar>(y-1,x-1))  -2* (bv.at<uchar>(y-1,x))  - (bv.at<uchar>(y-1,x+1)) ;
                dx /= 4;
                dy /=4;
                //cout << "val " << dx << endl;
                float deltaI = sqrt( dx*dx + dy*dy);
                if(deltaI>0)
                {
                    timer t;
                    t.start();
                    float d = x*dx + y*dy;
                    float rho = fabs(d/deltaI);
                    int index_rho = (int)trunc(rho);
                    float poids_rho = 1 - rho + index_rho;
                    float theta;
                    float alpha;
                    float dist_sign = x*dx + y * dy;

                    // Calcul de l'argument du gradient
                    if (dx)
                        alpha = atan(dy/dx);
                    else
                        alpha = M_PI/2;
                    // Calcul de theta, l'angle entre Ox et la droite
                    // passant par O et perpendiculaire à la droite D
                    if ((alpha < 0)&&(dy*dist_sign > 0))
                        theta = M_PI + alpha;
                    else
                        theta = alpha;

                    //Prendre les coordonnees
                    float pos_theta = ((theta + M_PI)*(T_theta-1))/(2*M_PI);
                    int index_theta = (int)(trunc(pos_theta));
                    {
                        float poids_theta =  1 - pos_theta + index_theta;
                        float vote_total = deltaI;
                        //#pragma omp critical
                        {
                            t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                            int ind = index_rho*T_theta + index_theta;
                            //nombre_vote[ind]++;
                            if (poids_rho < 1)
                            {
                                t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                                ind = (index_rho+1)*T_theta + index_theta;
                                //nombre_vote[ind]++;
                            }
                            if (poids_theta < 1)
                            {
                                t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                                ind = index_rho*T_theta + index_theta+1;
                                //nombre_vote[ind]++;
                            }
                            if ((poids_rho < 1)&&(poids_theta<1))
                            {
                                t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                                ind = (index_rho+1)*T_theta + index_theta+1;
                                //nombre_vote[ind]++;
                            }
                        }
                    }
                    t.end();
                    //execution_time+= t.ns()/1000.0;
                }
                grad_img.at<uchar>(y,x) =  uchar(deltaI);
            }
            else
            {
                grad_img.at<uchar>(y,x) = 0;
            }
        }
    }

    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);

    //#pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;

#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>100)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }

            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;

    std::list<vfloat3> list_temp;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = temp_acc[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( temp_acc[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = temp_acc[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == temp_acc[rho*T_theta + theta] && max > 100)
            {
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
                //cout << "max aa " << t_accumulator[rho*T_theta + theta] << " et rho " << rho << " et theta " << theta <<  endl;
            }
        }
    }



    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});

    Mat result;
    cvtColor(grad_img,result,CV_GRAY2BGR);

    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        if(nb_lines>100)
            break;
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPoints  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<12 && fabs(val[1]-coord[1])<15)
            {
                found = 1;
                break;
            }
        }
        if(found==0)
        {
            interestedPoints.push_back(coord);
            int ind = coord[0]*T_theta + coord[1];
            int x1,y1,x2,y2;
            vint4 ligne = getLineFromPoint(coord[0],coord[1],T_theta,nrows,ncols);
            x1 = ligne[0];
            y1 = ligne[1];
            x2 = ligne[2];
            y2 = ligne[3];
            cv::line(result, cv::Point(x1, y1), cv::Point(x2, y2), Scalar(0,255,255),3);
            nb_lines++;
        }
    }

    //cout << " nblines " << nb_lines << endl;

    //cv::imwrite("okay.bmp", result);
    return interestedPoints;
}

/**
 * @brief Hough_Lines_Parallel_one
 * @param img
 * @param bv
 * @param t_accumulator
 * @param Theta_max
 * @param max_of_the_accu
 * @param grad_img
 * @param list_interest
 * @return
 */
std::list<vint2> Hough_Lines_Parallel_one(image2d<uchar> img, Mat bv,
                                          std::vector<float>& t_accumulator,
                                          int Theta_max, float& max_of_the_accu
                                          ,  Mat &grad_img , std::vector<cv::LineIterator> list_interest)
{

    int ncols = img.ncols();
    int nrows = img.nrows();
    int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
    float T_theta = Theta_max;
    std::list<vint2> interestedPoints;

    for(int h = 0 ; h < list_interest.size() ; h++)
    {
        cv::LineIterator it = list_interest[h];
        for(int i = 0; i < it.count; i++, it++)
        {
            cv::Point curr = it.pos();
            int x_ = curr.x;
            int y_ = curr.y;
            for(int y = y_-5 ;y<=y_+5;y++)
            {
#pragma omp parallel for
                for(int x = x_-5; x <= x_+5 ;x++)
                {
                    if(x>3 && y>3 && x<ncols-3 && y<nrows-3)
                    {
                        float dx = 0;
                        float dy = 0;
                        dx = -(bv.at<uchar>(y+1,x-1))  +  (bv.at<uchar>(y+1,x+1))
                                -2* (bv.at<uchar>(y,x-1))  +  2*(bv.at<uchar>(y,x+1))
                                - (bv.at<uchar>(y-1,x-1))  + (bv.at<uchar>(y-1,x+1)) ;
                        dy = (bv.at<uchar>(y+1,x-1))  + 2*(bv.at<uchar>(y+1,x)) + (bv.at<uchar>(y+1,x+1))
                                - (bv.at<uchar>(y-1,x-1))  -2* (bv.at<uchar>(y-1,x))  - (bv.at<uchar>(y-1,x+1)) ;
                        dx /= 4;
                        dy /=4;
                        //cout << "val " << dx << endl;
                        float deltaI = sqrt( dx*dx + dy*dy);
                        if(deltaI>100)
                        {
                            float d = x*dx + y*dy;
                            float rho = fabs(d/deltaI);
                            int index_rho = (int)trunc(rho);
                            float poids_rho = 1 - rho + index_rho;
                            float theta;
                            float alpha;
                            float dist_sign = x*dx + y * dy;

                            // Calcul de l'argument du gradient
                            if (dx)
                                alpha = atan(dy/dx);
                            else
                                alpha = M_PI/2;
                            // Calcul de theta, l'angle entre Ox et la droite
                            // passant par O et perpendiculaire à la droite D
                            if ((alpha < 0)&&(dy*dist_sign > 0))
                                theta = M_PI + alpha;
                            else
                                theta = alpha;
                            //Prendre les coordonnees
                            float pos_theta = ((theta + M_PI)*(T_theta-1))/(2*M_PI);
                            int index_theta = (int)(trunc(pos_theta));
                            {
                                float poids_theta =  1 - pos_theta + index_theta;
                                float vote_total = deltaI;
                                //#pragma omp critical
                                {
                                    t_accumulator[index_rho*T_theta + index_theta] += vote_total*poids_rho*poids_theta;
                                    int ind = index_rho*T_theta + index_theta;
                                    //nombre_vote[ind]++;
                                    if (poids_rho < 1)
                                    {
                                        t_accumulator[(index_rho+1)*T_theta + index_theta] += vote_total*(1-poids_rho)*poids_theta;
                                        ind = (index_rho+1)*T_theta + index_theta;
                                        //nombre_vote[ind]++;
                                    }
                                    if (poids_theta < 1)
                                    {
                                        t_accumulator[index_rho*T_theta + index_theta+1] += vote_total*poids_rho*(1-poids_theta);
                                        ind = index_rho*T_theta + index_theta+1;
                                        //nombre_vote[ind]++;
                                    }
                                    if ((poids_rho < 1)&&(poids_theta<1))
                                    {
                                        t_accumulator[(index_rho+1)*T_theta + index_theta+1] += vote_total*(1-poids_rho)*(1-poids_theta);
                                        ind = (index_rho+1)*T_theta + index_theta+1;
                                        //nombre_vote[ind]++;
                                    }
                                }
                            }
                        }
                        grad_img.at<uchar>(y,x) =  uchar(deltaI);
                    }
                    else
                        grad_img.at<uchar>(y,x) =  uchar(0);
                }
            }
        }
    }


    std::vector<float> temp_acc(rhomax*T_theta);
    std::vector<float> temp_acc_t(rhomax*T_theta);



    //#pragma omp parallel for
    for(int i =0;i<t_accumulator.size();i++)
    {
        temp_acc[i] = t_accumulator[i];
        temp_acc_t[i] = t_accumulator[i];
    }

    max_of_the_accu = 0;


#pragma omp parallel for
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            int comp = 0;
            if(temp_acc[rho*T_theta+theta]>50)
            {
                for(int i = rho-12;i<=rho+12;i++)
                {
                    for(int j = theta-15; j <= theta+15 ;j++)
                    {
                        if(!( i < 0 || i > rhomax || j <0 || j>T_theta))
                        {
                            if(temp_acc[rho*T_theta+theta]<temp_acc_t[i*T_theta+j])
                            {
                                comp++;
                                break;
                            }
                        }
                    }
                    if(comp>0)
                        break;
                }
            }
            else
            {
                comp++;
            }

            if(comp>0)
                temp_acc[rho*T_theta+theta] = 0;
        }
    }

    temp_acc_t = temp_acc;

    std::list<vfloat3> list_temp;

    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            float max = temp_acc[rho*T_theta + theta];
            if(max==0)
                continue;
            for(int ly=-4;ly<=4;ly++)
            {
                for(int lx=-4;lx<=4;lx++)
                {
                    if( (ly+rho>=0 && ly+rho<rhomax) && (lx+theta>=0 && lx+theta<T_theta) )
                    {
                        if( temp_acc[( (rho+ly)*T_theta) + (theta+lx)] > max )
                        {
                            max = temp_acc[( (rho+ly)*T_theta) + (theta+lx)];
                            ly = lx = 5;
                        }
                    }
                }
            }
            if(max == temp_acc[rho*T_theta + theta] && max > 500)
            {
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
                //cout << "max aa " << t_accumulator[rho*T_theta + theta] << " et rho " << rho << " et theta " << theta <<  endl;
            }
        }
    }


    list_temp.sort( [&](vfloat3& a, vfloat3& b){return a[0] > b[0];});

    Mat result;
    cvtColor(grad_img,result,CV_GRAY2BGR);

    int nb_lines = 0;
    for ( auto& x : list_temp )
    {
        if(nb_lines==0)
            max_of_the_accu = x[0];
        if(nb_lines>300)
            break;
        vint2 coord;
        coord = vint2(x[1],x[2]);
        int found = 0;
        for(auto& it : interestedPoints  )
        {
            vint2 val = it;
            if(fabs(val[0]-coord[0])<12 && fabs(val[1]-coord[1])<15)
            {
                found = 1;
                break;
            }
        }
        if(found==0)
        {
            interestedPoints.push_back(coord);
            int ind = coord[0]*T_theta + coord[1];
            int x1,y1,x2,y2;
            vint4 ligne = getLineFromPoint(coord[0],coord[1],T_theta,nrows,ncols);
            x1 = ligne[0];
            y1 = ligne[1];
            x2 = ligne[2];
            y2 = ligne[3];
            cv::line(result, cv::Point(x1, y1), cv::Point(x2, y2), (0,255,255),2);
            nb_lines++;
        }
    }
    cv::imwrite("okay.bmp", result);
    return interestedPoints;
}


/**
 * @brief getVectorVal
 * @param t_array
 * @param vert
 * @param hori
 * @param i
 * @param j
 * @return
 */
float get_vector_val(std::vector<float> t_array, int vert,int hori,int i ,int j)
{
    if(i<0 || i>=vert || j<0 || j>=hori )
        return 0;
    else
        return t_array[i*hori+j];
}


/**
 * @brief adap_thresold
 * @param list_temp
 * @param threshold_hough
 * @param calls
 * @param nb_calls_limits_reached
 * @param rhomax
 * @param T_theta
 * @param t_accumulator
 */
void adap_thresold(std::list<vfloat3> &list_temp , float &threshold_hough , int &calls ,
                   int &nb_calls_limits_reached , int rhomax, int T_theta , std::vector<float> t_accumulator)
{
    if(calls>=5)
    {
        nb_calls_limits_reached=1;
        return;
    }
    if(list_temp.size() < 100 && list_temp.size() >50)
    {
        return;
    }
    else if(list_temp.size() > 100 )
    {
        calls++;
        threshold_hough *= calls;
        reduce_number_of_max_local(list_temp,threshold_hough,rhomax,T_theta,t_accumulator);
    }
    else if(list_temp.size())
    {
        calls++;
        threshold_hough /=calls;
        reduce_number_of_max_local(list_temp,threshold_hough,rhomax,T_theta,t_accumulator);
    }
}

/**
 * @brief reduce_number_of_max_local
 * @param list_temp
 * @param threshold_hough
 * @param rhomax
 * @param T_theta
 * @param t_accumulator
 */
void reduce_number_of_max_local(std::list<vfloat3> &list_temp , float threshold_hough , int rhomax, int T_theta , std::vector<float> t_accumulator)
{
    list_temp.clear();
    for(int rho = 0 ; rho < rhomax ; rho ++ )
    {
        for(int theta = 0 ; theta < T_theta ; theta++)
        {
            if(t_accumulator[rho*T_theta + theta]>threshold_hough)
            {
                list_temp.push_back(vfloat3(t_accumulator[rho*T_theta + theta], rho,theta));
            }
        }
    }
}


/**
 * @brief interpolate_acculator
 * @param acc
 * @param seuil
 */
void interpolate_acculator(image2d<float> &acc, float seuil)
{
    image2d<float> out(acc.domain());
    pixel_wise(out, relative_access(acc), acc.domain()) | [&] (auto& o, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>3 && y>3 && x<acc.ncols()-3 && y<acc.ncols()-3 && i(0,0) > seuil)
        {
            o = i(0,0) + i(0,1) + i(0,-1) + i(1,0) ;
        }
    };
}


/**
 * @brief Hough_Lines_Parallel_Map
 * @param img
 */
void Hough_Lines_Parallel_Map(image2d<vuchar1> img)
{
    std::map<int,float> accumulator_high;
    std::list<vfloat2> list_accumulator;
    float T_theta = 100;
    int ncols = img.ncols();
    int nrows = img.nrows();
    image2d<vuchar1> out(img.domain());
    int cp = 0;
    int nb_cluster = 0;
    pixel_wise(out, relative_access(img), img.domain()) | [&] (auto& o, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(x>3 && y>3 && x<ncols-3 && y<nrows-3)
        {
            float dx = -(i(1,-1)).coeffRef(0)  +  (i(1,1)).coeffRef(0)
                    -2* (i(0,-1)).coeffRef(0)  +  2*(i(0,1)).coeffRef(0)
                    - (i(-1,-1)).coeffRef(0)  + (i(-1,1)).coeffRef(0) ;
            float dy = (i(1,-1)).coeffRef(0)  + 2*(i(1,0)).coeffRef(0)  + (i(1,1)).coeffRef(0)
                    - (i(-1,-1)).coeffRef(0)  -2* (i(-1,0)).coeffRef(0)  - (i(-1,1)).coeffRef(0) ;
            dx /= 4;
            dy /=4;
            float deltaI = sqrt( dx*dx + dy*dy);
            if(deltaI>125)
            {
                float d = x*dx + y*dy;
                float rho = fabs(d/deltaI);
                int index_rho = (int)trunc(rho);
                //float poids_rho = 1 - rho + index_rho;
                float theta;
                if(dx*dy<0 && d*dy>0)
                    theta = M_PI + atan(dy/dx);
                else
                    theta = atan(dy/dx);
                float pos_theta = ((theta + M_PI)*(T_theta-1))/(2*M_PI);
                int index_theta = (int)(trunc(pos_theta));
                //float poids_theta =  1 - pos_theta + index_theta;
#pragma omp critical
                {
                    int k = 0.5 * (index_rho + index_theta) * ( index_rho + index_theta + 1 ) + index_theta;
                    float value=0;
                    auto s = accumulator_high.find(k);
                    if(s!=accumulator_high.end())
                    {
                        value = s->second;
                    }
                    //accumulator_high[k] = deltaI*poids_rho*poids_theta + value;
                    accumulator_high[k] = deltaI + value;
                    nb_cluster++;
                }
            }
            o = vuchar1(uchar(round(deltaI)));
        }
        else
            o = vuchar1(0);
        ++cp;
    };
    cv::imwrite("sortieP.jpg", to_opencv(out));
    Mat result;
    cvtColor(to_opencv(out),result,CV_GRAY2BGR);
    int taille_map = accumulator_high.size();

    for ( auto const &val : accumulator_high)
    {
        vfloat2 vg(val.first,val.second);
        list_accumulator.push_back(vg);
    }
    list_accumulator.sort( [&](vfloat2& a, vfloat2& b){return a[1] > b[1];});

    int lines_taced = 0;
    std::vector<vint2> liste_ligne;
    //
    //#pragma omp parallel
    for ( auto& x : list_accumulator )
    {
        if(lines_taced>100)
            break;
        int k = x[0];
        vint2 coord = inverse_cantor<float>(k);
        //cout << " boule " << coord << endl;
        int found = 0;
        //cout << "affiche" << endl;
        for(int i = 0; i < liste_ligne.size() ; i++  )
        {
            vint2 val = liste_ligne[i];
            //cout << val << endl << endl;
            if(fabs(val[0]-coord[0])<5 && fabs(val[1]-coord[1])<5)
            {   //cout << " trouve " << coord << endl << endl ;
                found = 1;
                break;
            }
        }

        if(found==0 && x[1]>1000)
        {
#pragma omp critical
            liste_ligne.push_back(coord);
            //cout << "taille actuelle " << liste_ligne.size() << endl;
            int rho = coord[0];
            int theta = coord[1];
            int x1,x2,y1,y2;
            x1=x2=y1=y2=0;
            //intersection avec l'axe des x
            float t = ((2*M_PI*(theta))/ (T_theta-1)) - M_PI;
            float r = rho;

            y1=0;
            float cosinus = cos(t);
            if(fabs(cosinus)>0.01)
            {
                x1=int(r/cosinus);
                if(cosinus<0)
                {
                    for(int b = nrows -1 ; b >=0 ; b--)
                    {
                        x1 = (int)round((rho - b*sin(t))/cos(t));
                        if(x1>0)
                        {
                            y1 = b;
                            break;
                        }
                    }
                }
            }
            //intersection avec l'axe des y
            x2=0;
            float sinus = sin(t);
            if(fabs(sinus) < 0.01)
            {
                x2 = x1;
                y2 = nrows-1;
            }
            else
            {
                y2=int(r/sinus);
                if(y2<0)
                {
                    for(int a = ncols-1 ; a>=0 ; a--)
                    {
                        y2 = (int)round((rho - a*cos(t))/sin(t));
                        if(y2>0)
                        {
                            x2 = a;
                            break;
                        }
                    }
                }
            }
            //cout << " rho " << r << " theta " << t << " val " << x[1] <<  endl;
            if(fabs(cosinus)<0.01)
            {
                x1 = ncols-1;
                y1 = y2;
                //continue;
            }
            cv::line(result, cv::Point(x1, y1), cv::Point(x2, y2), (0,255,255),1);
            lines_taced++;
        }
    }
    /*for (int i=0;i < liste_ligne.size();i++)
        cout << liste_ligne[i] << endl << endl;*/
    //cout << " taille arbre " << taille_map << endl;
    cout << "nombre" << lines_taced << endl;
    cv::imwrite("okay.jpg", result);
}

}

