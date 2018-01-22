
#include "structure_from_motion.hh"

namespace vpp
{

//from http://www.mip.informatik.uni-kiel.de/tiki-index.php?page=Lilian+Zhang

void pose_estimation_from_line_correspondence(Eigen::MatrixXf start_points,
                                              Eigen::MatrixXf end_points,
                                              Eigen::MatrixXf directions,
                                              Eigen::MatrixXf points,
                                              Eigen::MatrixXf &rot_cw,
                                              Eigen::VectorXf &pos_cw)
{


    int n = start_points.cols();
    if(n != directions.cols())
    {
        return;
    }

    if(n<4)
    {
        return;
    }


    float condition_err_threshold = 1e-3;
    Eigen::VectorXf cosAngleThreshold(3);
    cosAngleThreshold << 1.1, 0.9659, 0.8660;
    Eigen::MatrixXf optimumrot_cw(3,3);
    Eigen::VectorXf optimumpos_cw(3);
    std::vector<float> lineLenVec(n,1);

    vfloat3 l1;
    vfloat3 l2;
    vfloat3 nc1;
    vfloat3 Vw1;
    vfloat3 Xm;
    vfloat3 Ym;
    vfloat3 Zm;
    Eigen::MatrixXf Rot(3,3);
    std::vector<vfloat3> nc_bar(n,vfloat3(0,0,0));
    std::vector<vfloat3> Vw_bar(n,vfloat3(0,0,0));
    std::vector<vfloat3> n_c(n,vfloat3(0,0,0));
    Eigen::MatrixXf Rx(3,3);
    int line_id;

    for(int HowToChooseFixedTwoLines = 1 ; HowToChooseFixedTwoLines <=3 ; HowToChooseFixedTwoLines++)
    {

        if(HowToChooseFixedTwoLines==1)
        {
#pragma omp parallel for
            for(int i = 0; i < n ; i++ )
            {
                // to correct
                float lineLen = 10;
                lineLenVec[i] = lineLen;
            }
            std::vector<float>::iterator result;
            result = std::max_element(lineLenVec.begin(), lineLenVec.end());
            line_id = std::distance(lineLenVec.begin(), result);
            vfloat3 temp;
            temp = start_points.col(0);
            start_points.col(0) = start_points.col(line_id);
            start_points.col(line_id) = temp;

            temp = end_points.col(0);
            end_points.col(0) = end_points.col(line_id);
            end_points.col(line_id) = temp;

            temp = directions.col(line_id);
            directions.col(0) = directions.col(line_id);
            directions.col(line_id) = temp;

            temp = points.col(0);
            points.col(0) = points.col(line_id);
            points.col(line_id) = temp;

            lineLenVec[line_id] = lineLenVec[1];
            lineLenVec[1] = 0;
            l1 = start_points.col(0) - end_points.col(0);
            l1 = l1/l1.norm();
        }


        for(int i = 1; i < n; i++)
        {
            std::vector<float>::iterator result;
            result = std::max_element(lineLenVec.begin(), lineLenVec.end());
            line_id = std::distance(lineLenVec.begin(), result);
            l2 = start_points.col(line_id) - end_points.col(line_id);
            l2 = l2/l2.norm();
            lineLenVec[line_id] = 0;
            MatrixXf cosAngle(3,3);
            cosAngle = (l1.transpose()*l2).cwiseAbs();
            if(cosAngle.maxCoeff() < cosAngleThreshold[HowToChooseFixedTwoLines])
            {
                break;
            }
        }



        vfloat3 temp;
        temp = start_points.col(1);
        start_points.col(1) = start_points.col(line_id);
        start_points.col(line_id) = temp;

        temp = end_points.col(1);
        end_points.col(1) = end_points.col(line_id);
        end_points.col(line_id) = temp;

        temp = directions.col(1);
        directions.col(1) = directions.col(line_id);
        directions.col(line_id) = temp;

        temp = points.col(1);
        points.col(1) = points.col(line_id);
        points.col(line_id) = temp;

        lineLenVec[line_id] = lineLenVec[1];
        lineLenVec[1] = 0;

        // The rotation matrix R_wc is decomposed in way which is slightly different from the description in the paper,
        // but the framework is the same.
        // R_wc = (Rot') * R * Rot =  (Rot') * (Ry(theta) * Rz(phi) * Rx(psi)) * Rot
        nc1 = x_cross(start_points.col(1),end_points.col(1));
        nc1 = nc1/nc1.norm();

        Vw1 = directions.col(1);
        Vw1 = Vw1/Vw1.norm();

        //the X axis of Model frame
        Xm = x_cross(nc1,Vw1);
        Xm = Xm/Xm.norm();

        //the Y axis of Model frame
        Ym = nc1;

        //the Z axis of Model frame
        Zm = x_cross(Xm,Zm);
        Zm = Zm/Zm.norm();

        //Rot * [Xm, Ym, Zm] = I.
        Rot.col(0) = Xm;
        Rot.col(1) = Ym;
        Rot.col(2) = Zm;

        Rot = Rot.transpose();


        //rotate all the vector by Rot.
        //nc_bar(:,i) = Rot * nc(:,i)
        //Vw_bar(:,i) = Rot * Vw(:,i)
#pragma omp parallel for
        for(int i = 0 ; i < n ; i++)
        {
            vfloat3 nc = x_cross(start_points.col(1),end_points.col(1));
            nc = nc/nc.norm();
            n_c[i] = nc;
            nc_bar[i] = Rot * nc;
            Vw_bar[i] = Rot * directions.col(i);
        }

        //Determine the angle psi, it is the angle between z axis and Vw_bar(:,1).
        //The rotation matrix Rx(psi) rotates Vw_bar(:,1) to z axis
        float cospsi = (Vw_bar[1])[2];//the angle between z axis and Vw_bar(:,1); cospsi=[0,0,1] * Vw_bar(:,1);.
        float sinpsi= sqrt(1 - cospsi*cospsi);
        Rx.row(0) = vfloat3(1,0,0);
        Rx.row(1) = vfloat3(0,cospsi,-sinpsi);
        Rx.row(2) = vfloat3(0,sinpsi,cospsi);
        vfloat3 Zaxis = Rx * Vw_bar[1];
        if(1-fabs(Zaxis[3]) > 1e-5)
        {
            Rx = Rx.transpose();
        }

        //estimate the rotation angle phi by least square residual.
        //i.e the rotation matrix Rz(phi)
        vfloat3 Vm2 = Rx * Vw_bar[1];
        float A2 = Vm2[0];
        float B2 = Vm2[1];
        float C2 = Vm2[2];
        float x2 = (nc_bar[1])[0];
        float y2 = (nc_bar[1])[1];
        float z2 = (nc_bar[1])[2];
        Eigen::PolynomialSolver<double, Eigen::Dynamic> solver;
        Eigen::VectorXf coeff(9);

        std::vector<float> coef(9,0); //coefficients of equation (7)
        Eigen::VectorXf polyDF = VectorXf::Zero(16); //%dF = ployDF(1) * t^15 + ployDF(2) * t^14 + ... + ployDF(15) * t + ployDF(16);

        //construct the  polynomial F'
#pragma omp parallel for
        for(int i = 3 ; i < n ; i++)
        {
            vfloat3 Vm3 = Rx*Vw_bar[i];
            float A3 = Vm3[0];
            float B3 = Vm3[1];
            float C3 = Vm3[2];
            float x3 = (nc_bar[i])[0];
            float y3 = (nc_bar[i])[1];
            float z3 = (nc_bar[i])[2];
            float u11 = -z2*A2*y3*B3 + y2*B2*z3*A3;
            float u12 = -y2*A2*z3*B3 + z2*B2*y3*A3;
            float u13 = -y2*B2*z3*B3 + z2*B2*y3*B3 + y2*A2*z3*A3 - z2*A2*y3*A3;
            float u14 = -y2*B2*x3*C3 + x2*C2*y3*B3;
            float u15 =  x2*C2*y3*A3 - y2*A2*x3*C3;
            float u21 = -x2*A2*y3*B3 + y2*B2*x3*A3;
            float u22 = -y2*A2*x3*B3 + x2*B2*y3*A3;
            float u23 =  x2*B2*y3*B3 - y2*B2*x3*B3 - x2*A2*y3*A3 + y2*A2*x3*A3;
            float u24 =  y2*B2*z3*C3 - z2*C2*y3*B3;
            float u25 =  y2*A2*z3*C3 - z2*C2*y3*A3;
            float u31 = -x2*A2*z3*A3 + z2*A2*x3*A3;
            float u32 = -x2*B2*z3*B3 + z2*B2*x3*B3;
            float u33 =  x2*A2*z3*B3 - z2*A2*x3*B3 + x2*B2*z3*A3 - z2*B2*x3*A3;
            float u34 =  z2*A2*z3*C3 + x2*A2*x3*C3 - z2*C2*z3*A3 - x2*C2*x3*A3;
            float u35 = -z2*B2*z3*C3 - x2*B2*x3*C3 + z2*C2*z3*B3 + x2*C2*x3*B3;
            float u36 = -x2*C2*z3*C3 + z2*C2*x3*C3;
            float a4 =   u11*u11 + u12*u12 - u13*u13 - 2*u11*u12 +   u21*u21 + u22*u22 - u23*u23
                    -2*u21*u22 - u31*u31 - u32*u32 +   u33*u33 + 2*u31*u32;
            float a3 =2*(u11*u14 - u13*u15 - u12*u14 +   u21*u24 -   u23*u25
                         - u22*u24 - u31*u34 + u33*u35 +   u32*u34);
            float a2 =-2*u12*u12 + u13*u13 + u14*u14 -   u15*u15 + 2*u11*u12 - 2*u22*u22 + u23*u23
                    + u24*u24 - u25*u25 +2*u21*u22+ 2*u32*u32 -   u33*u33
                    - u34*u34 + u35*u35 -2*u31*u32- 2*u31*u36 + 2*u32*u36;
            float a1 =2*(u12*u14 + u13*u15 +  u22*u24 +  u23*u25 -   u32*u34 - u33*u35 - u34*u36);
            float a0 =   u12*u12 + u15*u15+   u22*u22 +  u25*u25 -   u32*u32 - u35*u35 - u36*u36 - 2*u32*u36;
            float b3 =2*(u11*u13 - u12*u13 +  u21*u23 -  u22*u23 -   u31*u33 + u32*u33);
            float b2 =2*(u11*u15 - u12*u15 +  u13*u14 +  u21*u25 -   u22*u25 + u23*u24 - u31*u35 + u32*u35 - u33*u34);
            float b1 =2*(u12*u13 + u14*u15 +  u22*u23 +  u24*u25 -   u32*u33 - u34*u35 - u33*u36);
            float b0 =2*(u12*u15 + u22*u25 -  u32*u35 -  u35*u36);

            float d0 =    a0*a0 -   b0*b0;
            float d1 = 2*(a0*a1 -   b0*b1);
            float d2 =    a1*a1 + 2*a0*a2 +   b0*b0 - b1*b1 - 2*b0*b2;
            float d3 = 2*(a0*a3 +   a1*a2 +   b0*b1 - b1*b2 -   b0*b3);
            float d4 =    a2*a2 + 2*a0*a4 + 2*a1*a3 + b1*b1 + 2*b0*b2 - b2*b2 - 2*b1*b3;
            float d5 = 2*(a1*a4 +   a2*a3 +   b1*b2 + b0*b3 -   b2*b3);
            float d6 =    a3*a3 + 2*a2*a4 +   b2*b2 - b3*b3 + 2*b1*b3;
            float d7 = 2*(a3*a4 +   b2*b3);
            float d8 =    a4*a4 +   b3*b3;
            std::vector<float> v { a4, a3, a2, a1, a0, b3, b2, b1, b0 };
            Eigen::VectorXf vp;
            vp <<  a4, a3, a2, a1, a0, b3, b2, b1, b0 ;
            //coef = coef + v;
            coeff = coeff + vp;

            polyDF[0] = polyDF[0] + 8*d8*d8;
            polyDF[1] = polyDF[1] + 15* d7*d8;
            polyDF[2] = polyDF[2] + 14* d6*d8 + 7*d7*d7;
            polyDF[3] = polyDF[3] + 13*(d5*d8 +  d6*d7);
            polyDF[4] = polyDF[4] + 12*(d4*d8 +  d5*d7) +  6*d6*d6;
            polyDF[5] = polyDF[5] + 11*(d3*d8 +  d4*d7 +  d5*d6);
            polyDF[6] = polyDF[6] + 10*(d2*d8 +  d3*d7 +  d4*d6) + 5*d5*d5;
            polyDF[7] = polyDF[7] + 9 *(d1*d8 +  d2*d7 +  d3*d6  + d4*d5);
            polyDF[8] = polyDF[8] + 8 *(d1*d7 +  d2*d6 +  d3*d5) + 4*d4*d4 + 8*d0*d8;
            polyDF[9] = polyDF[9] + 7 *(d1*d6 +  d2*d5 +  d3*d4) + 7*d0*d7;
            polyDF[10] = polyDF[10] + 6 *(d1*d5 +  d2*d4) + 3*d3*d3 + 6*d0*d6;
            polyDF[11] = polyDF[11] + 5 *(d1*d4 +  d2*d3)+ 5*d0*d5;
            polyDF[12] = polyDF[12] + 4 * d1*d3 +  2*d2*d2 + 4*d0*d4;
            polyDF[13] = polyDF[13] + 3 * d1*d2 +  3*d0*d3;
            polyDF[14] = polyDF[14] + d1*d1 + 2*d0*d2;
            polyDF[15] = polyDF[15] + d0*d1;
        }


        Eigen::VectorXd coefficientPoly = VectorXd::Zero(16);

        for(int j =0; j < 16 ; j++)
        {
            coefficientPoly[j] = polyDF[15-j];
        }


        //solve polyDF
        solver.compute(coefficientPoly);
        const Eigen::PolynomialSolver<double, Eigen::Dynamic>::RootsType & r = solver.roots();
        Eigen::VectorXd rs(r.rows());
        Eigen::VectorXd is(r.rows());
        std::vector<float> min_roots;
        for(int j =0;j<r.rows();j++)
        {
            rs[j] = fabs(r[j].real());
            is[j] = fabs(r[j].imag());
        }


        float maxreal = rs.maxCoeff();

        for(int j = 0 ; j < rs.rows() ; j++ )
        {
            if(is[j]/maxreal <= 0.001)
            {
                min_roots.push_back(rs[j]);
            }
        }

        std::vector<float> temp_v(15);
        std::vector<float> poly(15);
        for(int j = 0 ; j < 15 ; j++)
        {
            temp_v[j] = j+1;
        }

        for(int j = 0 ; j < 15 ; j++)
        {
            poly[j] = polyDF[j]*temp_v[j];
        }

        Eigen::Matrix<double,16,1> polynomial;

        Eigen::VectorXd evaluation(min_roots.size());

        for( int j = 0; j < min_roots.size(); j++ )
        {
            evaluation[j] = poly_eval( polynomial, min_roots[j] );
        }


        std::vector<float> minRoots;


        for( int j = 0; j < min_roots.size(); j++ )
        {
            if(!evaluation[j]<=0)
            {
                minRoots.push_back(min_roots[j]);
            }
        }


        if(minRoots.size()==0)
        {
            cout << "No solution" << endl;
            return;
        }

        int numOfRoots = minRoots.size();
        //for each minimum, we try to find a solution of the camera pose, then
        //choose the one with the least reprojection residual as the optimum of the solution.
        float minimalReprojectionError = 100;
        // In general, there are two solutions which yields small re-projection error
        // or condition error:"n_c * R_wc * V_w=0". One of the solution transforms the
        // world scene behind the camera center, the other solution transforms the world
        // scene in front of camera center. While only the latter one is correct.
        // This can easily be checked by verifying their Z coordinates in the camera frame.
        // P_c(Z) must be larger than 0 if it's in front of the camera.



        for(int rootId = 0 ; rootId < numOfRoots ; rootId++)
        {

            float cosphi = minRoots[rootId];
            float sign1 = sign_of_number(coeff[0] * pow(cosphi,4)
                    + coeff[1] * pow(cosphi,3) + coeff[2] * pow(cosphi,2)
                    + coeff[3] * cosphi + coeff[4]);
            float  sign2 = sign_of_number(coeff[5] * pow(cosphi,3)
                    + coeff[6] * pow(cosphi,2) + coeff[7] * cosphi   + coeff[8]);
            float sinphi= -sign1*sign2*sqrt(abs(1-cosphi*cosphi));
            Eigen::MatrixXf Rz(3,3);
            Rz.row(0) = vfloat3(cosphi,-sinphi,0);
            Rz.row(1) = vfloat3(sinphi,cosphi,0);
            Rz.row(2) = vfloat3(0,0,1);
            //now, according to Sec4.3, we estimate the rotation angle theta
            //and the translation vector at a time.
            Eigen::MatrixXf RzRxRot(3,3);
            RzRxRot = Rz*Rx*Rot;


            //According to the fact that n_i^C should be orthogonal to Pi^c and Vi^c, we
            //have: scalarproduct(Vi^c, ni^c) = 0  and scalarproduct(Pi^c, ni^c) = 0.
            //where Vi^c = Rwc * Vi^w,  Pi^c = Rwc *(Pi^w - pos_cw) = Rwc * Pi^w - pos;
            //Using the above two constraints to construct linear equation system Mat about
            //[costheta, sintheta, tx, ty, tz, 1].
            Eigen::MatrixXf Matrice(2*n-1,6);
#pragma omp parallel for
            for(int i = 0 ; i < n ; i++)
            {
                float nxi = (nc_bar[i])[0];
                float nyi = (nc_bar[i])[1];
                float nzi = (nc_bar[i])[2];
                Eigen::VectorXf Vm(3);
                Vm = RzRxRot * directions.col(i);
                float Vxi = Vm[0];
                float Vyi = Vm[1];
                float Vzi = Vm[2];
                Eigen::VectorXf Pm(3);
                Pm = RzRxRot * points.col(i);
                float Pxi = Pm(1);
                float Pyi = Pm(2);
                float Pzi = Pm(3);
                //apply the constraint scalarproduct(Vi^c, ni^c) = 0
                //if i=1, then scalarproduct(Vi^c, ni^c) always be 0
                if(i>1)
                {
                    Matrice(2*i-3, 0) = nxi * Vxi + nzi * Vzi;
                    Matrice(2*i-3, 1) = nxi * Vzi - nzi * Vxi;
                    Matrice(2*i-3, 5) = nyi * Vyi;
                }
                //apply the constraint scalarproduct(Pi^c, ni^c) = 0
                Matrice(2*i-2, 0) = nxi * Pxi + nzi * Pzi;
                Matrice(2*i-2, 1) = nxi * Pzi - nzi * Pxi;
                Matrice(2*i-2, 2) = -nxi;
                Matrice(2*i-2, 3) = -nyi;
                Matrice(2*i-2, 4) = -nzi;
                Matrice(2*i-2, 5) = nyi * Pyi;
            }

            //solve the linear system Mat * [costheta, sintheta, tx, ty, tz, 1]' = 0  using SVD,
            JacobiSVD<MatrixXf> svd(Matrice, ComputeThinU | ComputeThinV);
            Eigen::MatrixXf VMat = svd.matrixV();
            Eigen::VectorXf vec(2*n-1);
            //the last column of Vmat;
            vec = VMat.col(5);
            //the condition that the last element of vec should be 1.
            vec = vec/vec[5];
            //the condition costheta^2+sintheta^2 = 1;
            float normalizeTheta = 1/sqrt(vec[0]*vec[1]+vec[1]*vec[1]);
            float costheta = vec[0]*normalizeTheta;
            float sintheta = vec[1]*normalizeTheta;
            Eigen::MatrixXf Ry(3,3);
            Ry << costheta, 0, sintheta , 0, 1, 0 , -sintheta, 0, costheta;

            //now, we get the rotation matrix rot_wc and translation pos_wc
            Eigen::MatrixXf rot_wc(3,3);
            rot_wc = (Rot.transpose()) * (Ry * Rz * Rx) * Rot;
            Eigen::VectorXf pos_wc(3);
            pos_wc = - Rot.transpose() * vec.segment(2,4);

            //now normalize the camera pose by 3D alignment. We first translate the points
            //on line in the world frame Pw to points in the camera frame Pc. Then we project
            //Pc onto the line interpretation plane as Pc_new. So we could call the point
            //alignment algorithm to normalize the camera by aligning Pc_new and Pw.
            //In order to improve the accuracy of the aligment step, we choose two points for each
            //lines. The first point is Pwi, the second point is  the closest point on line i to camera center.
            //(Pw2i = Pwi - (Pwi'*Vwi)*Vwi.)
            Eigen::MatrixXf Pw2(3,n);
            Pw2.setZero();
            Eigen::MatrixXf Pc_new(3,n);
            Pc_new.setZero();
            Eigen::MatrixXf Pc2_new(3,n);
            Pc2_new.setZero();

            for(int i = 0 ; i < n ; i++)
            {
                vfloat3 nci = n_c[i];
                vfloat3 Pwi = points.col(i);
                vfloat3 Vwi = directions.col(i);
                //first point on line i
                vfloat3 Pci;
                Pci = rot_wc * Pwi + pos_wc;
                Pc_new.col(i) = Pci - (Pci.transpose() * nci) * nci;
                //second point is the closest point on line i to camera center.
                vfloat3 Pw2i;
                Pw2i = Pwi - (Pwi.transpose() * Vwi) * Vwi;
                Pw2.col(i) = Pw2i;
                vfloat3 Pc2i;
                Pc2i = rot_wc * Pw2i + pos_wc;
                Pc2_new.col(i) = Pc2i - ( Pc2i.transpose() * nci ) * nci;
            }

            MatrixXf XXc(Pc_new.rows(), Pc_new.cols()+Pc2_new.cols());
            XXc << Pc_new, Pc2_new;
            MatrixXf XXw(points.rows(), points.cols()+Pw2.cols());
            XXw << points, Pw2;
            int nm = points.cols()+Pw2.cols();
            cal_campose(XXc,XXw,nm,rot_wc,pos_wc);
            pos_cw = -rot_wc.transpose() * pos_wc;

            //check the condition n_c^T * rot_wc * V_w = 0;
            float conditionErr = 0;
            for(int i =0 ; i < n ; i++)
            {
                float val = ( (n_c[i]).transpose() * rot_wc * directions.col(i) );
                conditionErr = conditionErr + val*val;
            }

            if(conditionErr/n < condition_err_threshold || HowToChooseFixedTwoLines ==3)
            {
                //check whether the world scene is in front of the camera.
                int numLineInFrontofCamera = 0;
                if(HowToChooseFixedTwoLines<3)
                {
                    for(int i = 0 ; i < n ; i++)
                    {
                        vfloat3 P_c = rot_wc * (points.col(i) - pos_cw);
                        if(P_c[2]>0)
                        {
                            numLineInFrontofCamera++;
                        }
                    }
                }
                else
                {
                    numLineInFrontofCamera = n;
                }

                if(numLineInFrontofCamera > 0.5*n)
                {
                    //most of the lines are in front of camera, then check the reprojection error.
                    int reprojectionError = 0;
                    for(int i =0; i < n ; i++)
                    {
                        //line projection function
                        vfloat3 nc = rot_wc * x_cross(points.col(i) - pos_cw , directions.col(i));
                        float h1 = nc.transpose() * start_points.col(i);
                        float h2 = nc.transpose() * end_points.col(i);
                        float lineLen = (start_points.col(i) - end_points.col(i)).norm()/3;
                        reprojectionError += lineLen * (h1*h1 + h1*h2 + h2*h2) / (nc[0]*nc[0]+nc[1]*nc[1]);
                    }
                    if(reprojectionError < minimalReprojectionError)
                    {
                        optimumrot_cw = rot_wc.transpose();
                        optimumpos_cw = pos_cw;
                        minimalReprojectionError = reprojectionError;
                    }
                }
            }
        }
        if(optimumrot_cw.rows()>0)
        {
            break;
        }
    }
    pos_cw = optimumpos_cw;
    rot_cw = optimumrot_cw;
}

inline
void cal_campose(Eigen::MatrixXf XXc,Eigen::MatrixXf XXw,
                int n,Eigen::MatrixXf &R2,Eigen::VectorXf &t2)
{
    //A
    Eigen::MatrixXf X = XXw;
    //B
    Eigen::MatrixXf Y = XXc;
    Eigen::MatrixXf eyen(n,n);
    eyen = Eigen::MatrixXf::Identity(n,n);
    Eigen::MatrixXf ones(n,n);
    ones.setOnes();
    Eigen::MatrixXf K(n,n);
    K = eyen - ones/n;


    vfloat3 ux;
    for(int i =0; i < n; i++)
    {
        ux = ux + X.col(i);
    }
    ux = ux/n;
    vfloat3 uy;
    for(int i =0; i < n; i++)
    {
        uy = uy + Y.col(i);
    }
    uy = uy/n;
    Eigen::MatrixXf XK(3,n);
    XK = X*K;
    Eigen::MatrixXf XKarre(3,n);
    for(int i = 0 ; i < n ; i++)
    {
        XKarre(0,i) = XK(0,i)*XK(0,i);
        XKarre(1,i) = XK(1,i)*XK(1,i);
        XKarre(2,i) = XK(2,i)*XK(2,i);
    }
    Eigen::VectorXf sumXKarre(n);
    float sigmx2 = 0;
    for(int i = 0 ; i < n ; i++)
    {
        sumXKarre[i] = XKarre(0,i) + XKarre(1,i) + XKarre(2,i);
        sigmx2 += sumXKarre[i];
    }
    sigmx2 /=n;
    Eigen::MatrixXf SXY(3,3);
    SXY = Y*K*(X.transpose())/n;
    JacobiSVD<MatrixXf> svd(SXY, ComputeThinU | ComputeThinV);
    Eigen::MatrixXf S(3,3);
    S = Eigen::MatrixXf::Identity(3,3);

    if(SXY.determinant() < 0)
    {
        S(3,3) = -1;
    }

    R2 = svd.matrixU() * S * (svd.matrixV()).transpose();

    Eigen::MatrixXf D(3,3);
    D.setZero();

    for(int i = 0 ; i < svd.singularValues().size() ; i++)
    {
        D(i,i) = (svd.singularValues())[i];
    }

    float c2 = (D*S).trace()/sigmx2;
    t2 = uy - c2*R2*ux;

    vfloat3 Xx = R2.col(0);
    vfloat3 Yy = R2.col(1);
    vfloat3 Zz = R2.col(2);

    if((x_cross(Xx,Yy)-Zz).norm()>2e-2)
    {
        R2.col(2) = -Zz;
    }
}

inline
void r_and_t(MatrixXf &rot_cw, VectorXf &pos_cw,MatrixXf start_points, MatrixXf end_points,
             MatrixXf P1w,MatrixXf P2w,MatrixXf initRot_cw,VectorXf initPos_cw,
             int maxIterNum,float TerminateTh,int nargin)
{
    if(nargin<6)
    {
        return;
    }

    if(nargin<8)
    {
        maxIterNum = 8;
        TerminateTh = 1e-5;
    }

    int n = start_points.cols();

    if(n != end_points.cols() || n!= P1w.cols() || n!= P2w.cols())
    {
        return;
    }

    if(n<4)
    {
        return;
    }

    //first compute the weight of each line and the normal of
    //the interpretation plane passing through to camera center and the line

    VectorXf w = VectorXf::Zero(n);
    MatrixXf nc = MatrixXf::Zero(3,n);

    for(int i = 0 ; i < n ; i++)
    {
        //the weight of a line is the inverse of its image length
        w[i] = 1/(start_points.col(i)-end_points.col(i)).norm();
        vfloat3 v1 = start_points.col(i);
        vfloat3 v2 = end_points.col(i);
        vfloat3 temp = v1.cross(v2);
        nc.col(i) = temp/temp.norm();
    }

    MatrixXf rot_wc = initPos_cw.transpose();
    MatrixXf pos_wc = - initRot_cw.transpose() * initPos_cw;


    for(int iter = 1 ; iter < maxIterNum ; iter++)
    {
        //construct the equation (31)
        MatrixXf A = MatrixXf::Zero(6,7);
        MatrixXf C = MatrixXf::Zero(3,3);
        MatrixXf D = MatrixXf::Zero(3,3);
        MatrixXf F = MatrixXf::Zero(3,3);
        vfloat3 c_bar = vfloat3(0,0,0);
        vfloat3 d_bar = vfloat3(0,0,0);
        for(int i = 0 ; i < n ; i++)
        {
            //for first point on line
            vfloat3 Pi = rot_wc * P1w.col(i);
            vfloat3 Ni = nc.col(i);
            float wi = w[i];
            vfloat3 bi = Pi.cross(Ni);
            C = C + wi*Ni*Ni.transpose();
            D = D + wi*bi*bi.transpose();
            F = F + wi*Ni*bi.transpose();
            vfloat3 tempi = Pi + pos_wc;
            float scale = Ni.transpose() * tempi;
            scale *= wi;
            c_bar = c_bar + scale * Ni;
            d_bar = d_bar + scale*bi;
            //for second point on line
            Pi = rot_wc * P2w.col(i);
            Ni = nc.col(i);
            wi = w[i];
            bi = Pi.cross(Ni);
            C  = C + wi*Ni*Ni.transpose();
            D  = D + wi*bi*bi.transpose();
            F  = F + wi*Ni*bi.transpose();
            scale = (Ni.transpose() * (Pi + pos_wc));
            scale *= wi;
            c_bar = c_bar + scale * Ni;
            d_bar = d_bar + scale * bi;
        }
        A.block<3,3>(0,0) = C;
        A.block<3,3>(0,3) = F;
        (A.col(6)).segment(0,2) = c_bar;
        A.block<3,3>(3,0) = F.transpose();
        A.block<3,3>(2,2) = D;
        (A.col(6)).segment(3,5) = d_bar;
        //sovle the system by using SVD;
        JacobiSVD<MatrixXf> svd(A, ComputeThinU | ComputeThinV);
        VectorXf vec(7);
        //the last column of Vmat;
        vec = (svd.matrixV()).col(6);
        //the condition that the last element of vec should be 1.
        vec = vec/vec[6];

        //update the rotation and translation parameters;
        vfloat3 dT = vec.segment(0,2);
        vfloat3 dOmiga = vec.segment(3,5);
        MatrixXf rtemp(3,3);
        rtemp << 1, -dOmiga[2], dOmiga[1], dOmiga[2], 1, -dOmiga[1], -dOmiga[1], dOmiga[0], 1;
        rot_wc = rtemp * rot_wc;
        //newRot_wc = ( I + [dOmiga]x ) oldRot_wc
        //may be we can compute new R using rodrigues(r+dr)
        pos_wc = pos_wc + dT;

        if(dT.norm() < TerminateTh && dOmiga.norm() < 0.1*TerminateTh)
        {
            break;
        }
    }
    rot_cw = rot_wc.transpose();
    pos_cw = -rot_cw * pos_wc;
}

inline
vfloat3 x_cross(vfloat3 a,vfloat3 b)
{
    vfloat3 c;
    c[0] = a[1]*b[2]-a[2]*b[1];
    c[1] = a[2]*b[0]-a[0]*b[2];
    c[2] = a[0]*b[1]-a[1]*b[0];
    return c;
}



}


