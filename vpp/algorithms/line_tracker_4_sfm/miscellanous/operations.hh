#pragma once

#include <vpp/vpp.hh>
#include <list>
#include <iostream>
#include <iterator>
#include <random>

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <Eigen/Core>
#include "lookup_tables.hh"

using namespace vpp;
using namespace std;
using namespace cv;
using namespace Eigen;


#define PI_FLOAT     3.14159265f
#define PIBY2_FLOAT  1.5707963f

int nb_call_fast = 0;
int real_fast = 0;

// |error| < 0.005
inline
float atan2_approximation2( float y, float x )
{
    if ( x == 0.0f )
    {
        if ( y > 0.0f ) return PIBY2_FLOAT;
        if ( y == 0.0f ) return 0.0f;
        return -PIBY2_FLOAT;
    }
    float atan;
    float z = y/x;
    if ( fabs( z ) < 1.0f )
    {
        atan = z/(1.0f + 0.28f*z*z);
        if ( x < 0.0f )
        {
            if ( y < 0.0f ) return atan - PI_FLOAT;
            return atan + PI_FLOAT;
        }
    }
    else
    {
        atan = PIBY2_FLOAT - z/(z*z + 0.28f);
        if ( y < 0.0f ) return atan - PI_FLOAT;
    }
    return atan;
}

inline double FastArcTan(double x)
{
    return M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
}

inline float FastArcTan(float dx,float dy)
{
    float val = dy/dx;
    float fabsval = fabs(val);
    //nb_call_fast++;
    if(fabsval < 1)
    {
        //cout << "val " << val << endl;
        //real_fast++;
        return M_PI_4*val - val*(fabsval - 1)*(0.2447 + 0.0663*fabsval);
    }
    else
    {
        return atan(val);
    }
}

inline vint4 getLineFromPoint_Fast(float rho,float theta,int T_theta,int nrows,int ncols,float &cosinus,float &sinus)
{
    int x1,x2,y1,y2;
    x1=x2=y1=y2=0;
    //intersection avec l'axe des x
    float t = 0;
    t = ((2*M_PI*(theta))/ (T_theta-1)) - M_PI;
    float r = rho;
    float eps = 0.01;


    cosinus = cos(t);
    sinus = sin(t);

    if(fabs(sinus)>eps && fabs(cosinus)>eps)
    {
        y1=0;
        x1 = r/cosinus;
        //cout << "premier x1 " << x1 << "  y1 " << y1 << endl;
        if(x1<0 || x1>ncols)
        {
            //cout << "val " << x1 << endl;
            x1 = ncols - 1;
            y1 = -(cosinus/sinus)*x1 + r/sinus;
            //cout << "deuxieme x1 " << x1 << "  y1 " << y1 << endl << endl;
        }
        x2=0;
        y2 = r/sinus;
        if(y2<0 || y2 >nrows)
        {
            y2 = nrows - 1;
            x2 = -(sinus/cosinus)*y2 + r/cosinus;
        }
    }
    else if(fabs(sinus) < eps)
    {
        y1 = 0;
        x1 = r/cosinus;
        x2 = x1;
        y2 = nrows - 1;
    }
    else if(fabs(cosinus) <eps)
    {
        x1 = 0;
        y1 = r/sinus;
        y2 = y1;
        x2 = ncols - 1;
    }

    return vint4(x1,y1,x2,y2);
}



inline vint4 getLineFromPoint(float rho,float theta,int T_theta,int nrows,int ncols)
{
    int x1,x2,y1,y2;
    x1=x2=y1=y2=0;
    //intersection avec l'axe des x
    int rhomax = int(sqrt(pow(ncols,2)+pow(nrows,2)));
    float t = 0;
    t = ((2*M_PI*(theta))/ (T_theta-1)) - M_PI;
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
    //cout << "x1 " << x1 << " y1 " << y1 << " x2 " << x2 << " y2 " << y2 << endl;
    return vint4(x1,y1,x2,y2);
}

inline vint4 getLineFromPoint(cv::Mat &out,int rho,int theta,int T_theta,int nrows,int ncols,int pos)
{
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
    return vint4(x1,y1,x2,y2);
}


inline void addGaussianNoise(float stddev,float mean,image2d<uchar> &img)
{
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);

    pixel_wise(img) | [&] (auto &i) {
        float val = i + dist(generator);
        i = uchar(val);
    };
}

inline void changeLuminosity(int valeur,image2d<uchar> &img)
{
    pixel_wise(img) | [&] (auto &i) {
        float val = valeur + i;
        i = uchar(val);
    };
}


inline bool vector_contains(std::list<vint2> liste,vint2 val,float error)
{
    for(auto &l : liste)
    {
        if((l - val).norm() <= error  )
        {
            return true;
        }
    }
    return false;
}

template <typename Derived, typename Derived2 >
Derived conv2d(const MatrixBase<Derived>& I, const MatrixBase<Derived2> &kernel )
{
    Derived O = Derived::Zero(I.rows(),I.cols());

    typedef typename Derived::Scalar Scalar;
    typedef typename Derived2::Scalar Scalar2;

    int col=0,row=0;
    int KSizeX = kernel.rows();
    int KSizeY = kernel.cols();

    int limitRow = I.rows()-KSizeX;
    int limitCol = I.cols()-KSizeY;

    Derived2 block ;
    Scalar normalization = kernel.sum();
    if ( normalization < 1E-6 )
    {
        normalization=1;
    }
    for ( row = KSizeX; row < limitRow; row++ )
    {
        for ( col = KSizeY; col < limitCol; col++ )
        {
            Scalar b=(static_cast<Derived2>( I.block(row,col,KSizeX,KSizeY ) ).cwiseProduct(kernel)).sum();
            O.coeffRef(row,col) = b;
        }
    }

    return O/normalization;
}



float convolution(Matrix<uchar,3,3> blockPixel, Matrix<float,3,3> kernel)
{
    float normalisation = kernel.sum();
    if ( normalisation < 1E-6 )
    {
        normalisation=1;
    }
    float res = 0;
    for(int i = 0 ; i < 3 ; i++)
    {
        for(int j =0 ; j < 3 ; j++)
        {
            res += blockPixel.coeffRef(i,j)*kernel.coeffRef(i,j);
        }
    }
    return res/normalisation;
}


void gaussian_blur5(image2d<uchar> src,image2d<uchar> dest)
{
    /* start producing Gaussian filter kernel*/
    const double PI = M_PI ;
    double sigma =  2;
    const int kernalWidth=5;
    const int kernalHeight=5;

    float kernalArray[kernalWidth][kernalHeight];

    double total=0;

    //calculate each relavant value to neighour pixels and store it in 2d array
    for(int row=0;row<kernalWidth;row++){

        for(int col=0;col<kernalHeight;col++){

            float value=(1/(2*PI*pow(sigma,2)))*exp(-(pow(row-kernalWidth/2,2)+pow(col-kernalHeight/2,2))/(2*pow(sigma,2)));

            kernalArray[row][col]=value;

            total+=value;
        }
    }

    //Scale value in 2d array in to 1
    for(int row=0;row<kernalWidth;row++){
        for(int col=0;col<kernalHeight;col++){

            kernalArray[row][col]=kernalArray[row][col]/total;


        }
    }


    int verticleImageBound = (kernalHeight-1)/2;
    int horizontalImageBound = (kernalWidth-1)/2;

    pixel_wise(dest, relative_access(src), src.domain()) | [&] (auto& g, auto i, vint2 coord) {
        int x = coord[1];
        int y = coord[0];
        if(y>verticleImageBound && y <src.nrows()-verticleImageBound
                && x > horizontalImageBound && x < src.ncols()-horizontalImageBound)
        {
            float value=0.0;
            //multiply pixel value with corresponding gaussian kernal value
            value = i(2,-2)*kernalArray[0][0] + i(2,-1)*kernalArray[0][1] + i(2,0)*kernalArray[0][2] + i(2,1)*kernalArray[0][3] + i(2,2)*kernalArray[0][4]
                    +   i(1,-2)*kernalArray[1][0] + i(1,-1)*kernalArray[1][1] + i(1,0)*kernalArray[1][2] + i(1,1)*kernalArray[1][3] + i(1,2)*kernalArray[1][4]
                    +   i(0,-2)*kernalArray[2][0] + i(0,-1)*kernalArray[2][1] + i(0,0)*kernalArray[2][2] + i(0,1)*kernalArray[2][3] + i(0,2)*kernalArray[2][4]
                    +   i(-1,-2)*kernalArray[3][0] + i(-1,-1)*kernalArray[3][1] + i(-1,0)*kernalArray[3][2] + i(-1,1)*kernalArray[3][3] + i(-1,2)*kernalArray[3][4]
                    +   i(-2,-2)*kernalArray[4][0] + i(-2,-1)*kernalArray[4][1] + i(-2,0)*kernalArray[4][2] + i(-2,1)*kernalArray[4][3] + i(-2,2)*kernalArray[4][4];
            //grayScaleImage.at<uchar>(kRow+row-verticleImageBound,kCol+col-horizontalImageBound)
            //*kernalArray[kRow][kCol];
            //assign new values to central point
            g = cvRound(value);
        }
        else
        {
            g = i(0,0);
        }
    };
}

template <typename type_>
inline
vint2 inverse_cantor(type_ z)
{
    auto w = floor((sqrt(8 * z + 1) - 1)/2);
    auto t = (w*w + w) / 2;
    int y = int(z - t);
    int x = int(w -y);
    return vint2(x,y);
}

template <typename type_>
inline
type_ cantor(type_ k1,type_ k2)
{
    type_ k =  0.5 * (k1 + k2) * ( k1 + k2 + 1 ) + k2;
    return k;
}

inline
float sign_of_number(float number_)
{
    if(number_<0)
        return -1;
    else if(number_>0)
        return 1;
    else
        return 0;
}

inline
float sinus_lut(float angle)
{
    int indice;
    float sign_res = 1;
    float sign_of_angle = sign_of_number(angle);
    while(fabs(angle)>M_PI)
    {
        //cout << " angle " << angle << endl;
        angle = angle - (sign_of_angle)*2*M_PI;
    }
    if(fabs(angle-sign_of_angle*M_PI)<0.0001 || fabs(angle-sign_of_angle*0.01)<0.0001)
        return 0;
    else if(fabs(angle-sign_of_angle*M_PI_2)<0.0001)
        return sign_of_angle;
    if(angle<0)
    {
        sign_res = -1;
        angle = -angle;
    }
    if(angle>M_PI_2)
    {
        angle = M_PI - angle;
    }
    indice = int(512.0 * angle / M_PI_2);
    return sign_res*sin_lut[indice];
}

inline
float cosinus_lut(float angle)
{
    return sinus_lut(M_PI_2 - angle);
}




