///////////////////////////////////////////////////////////////////////////////
///  Copyright (c) Visualization Group                                        
///  Institute of Computer Graphics and Computer Aided Design                 
///  School of Software, Tsinghua University                                  
///                                                                           
///  Created by : Hongsen Liao 2013-03-05                                         
///                                                                           
///  File : DataManager\data_type.h                                                     
///////////////////////////////////////////////////////////////////////////////

#if !defined( EA_C0513EFB_DC3B_4013_B468_0D717CCFD97C__INCLUDED_ )
#define EA_C0513EFB_DC3B_4013_B468_0D717CCFD97C__INCLUDED_

#include <cmath>

typedef long long int64;
typedef unsigned char uchar;

class Point2D;
class Point3D;
class PointF2D;
class PointF3D;

typedef Point2D Vector2D;
typedef Point3D Vector3D;
typedef PointF2D VectorF2D;
typedef PointF3D VectorF3D;

//----------------------------------------------------------------------------------------------------------------------

// ColorF: 4-component color
class ColorF
{

public:

    float a;
    float b;
    float g;
    float r;

    ColorF( void ) { r = 0.0f; g = 0.0f; b = 0.0f; a = 0.0f; };
    ColorF( float red, float green, float blue ) { r = red; g = green; b = blue; a = 0.0f; };
    ColorF( float red, float green, float blue, float alpha ) { r = red; g = green; b = blue; a = alpha; };

    const ColorF& operator = ( const ColorF& color ) { r = color.r; g = color.g; b = color.b; a = color.a; return color; };

    static ColorF FromHsv( float h, float s, float v){
        // TODO(Liao Hongsen) : code from the internet

        ColorF color;
        if ( h > 1.0f ) h = 1.0f;
        if ( h < 0.0f ) h = 0.0f;
        float hue6 = h*6.0f;
        if ( hue6 > 5 ) hue6 = 5;
        int z = hue6;
        float f = hue6 - z;
        switch(int(h*6.0f))
        {
        case 0:
            color.r = 1.0f;
            color.g = f;
            color.b = 0.0f;
            break;
        case 1:
            color.r = 1.0f - f;
            color.g = 1.0f;
            color.b = 0.0f;
            break;
        case 2:
            color.r = 0.0f;
            color.g = 1.0f;
            color.b = f;
            break;
        case 3:
            color.r = 0.0f;
            color.g = 1.0f - f;
            color.b = 1.0f;
            break;
        case 4:
            color.r = f;
            color.g = 0.0f;
            color.b = 1.0f;
            break;
        case 5:
        default:
            color.r = 1.0f;
            color.g = 0.0f;
            color.b = 1.0f - f;
        }
        return color;
    }
};
// ! ColorF

// Point2D: 2d point with int precision
class Point2D
{

public:
    Point2D( void ) { p_[0] = 0; p_[1] = 0; };
    Point2D( int x, int y ) { p_[0] = x; p_[1] = y; };
    ~Point2D( void ) {};

    inline int x( void ) const { return p_[0]; };
    inline int y( void ) const { return p_[1]; };
    inline void SetX( int x ) { p_[0] = x; };
    inline void SetY( int y ) { p_[1] = y; };
    inline int* data() { return p_; }

    void operator += ( const Point2D& p ) { p_[0] += p.x(); p_[1] += p.y(); };
    void operator += ( int c ) { p_[0] += c; p_[1] += c; };
    void operator -= ( const Point2D& p ) { p_[0] -= p.x(); p_[1] -= p.y(); };
    void operator -= ( int c ) { p_[0] -= c; p_[1] -= c; };
    void operator *= ( int c ) { p_[0] *= c; p_[1] *= c; };
    void operator /= ( int c ) { p_[0] /= c; p_[1] /= c; };

    inline int& operator []( int index ) { return p_[index]; };

    const Point2D& operator = ( const Point2D& p ) { p_[0] = p.x(); p_[1] = p.y(); return p; };

    //
    friend inline bool operator == ( const Point2D& p1, const Point2D& p2 ){

        return ( ( p1.p_[0] == p2.p_[0] ) && ( p1.p_[1] == p2.p_[1] ) );
    };
    //

    //
    friend inline bool operator != ( const Point2D& p1, const Point2D& p2 ){

        return ( ( p1.p_[0] != p2.p_[0] ) || ( p1.p_[1] != p2.p_[1] ) );
    };
    //

    //
    friend inline const Point2D operator + ( const Point2D& p1, const Point2D& p2 ){

        return Point2D( p1.x() + p2.x(), p1.y() + p2.y() );
    };
    //

    // 
    friend inline const Point2D operator + ( const Point2D& p, int c ){

        return Point2D( p.x() + c, p.y() + c );
    };
    //

    //
    friend inline const Point2D operator - ( const Point2D& p1, const Point2D& p2 ){

        return Point2D( p1.x() - p2.x(), p1.y() - p2.y() );
    };
    //

    // 
    friend inline const Point2D operator - ( const Point2D& p, int c ){

        return Point2D( p.x() - c, p.y() - c );
    };
    //

    // 
    friend inline const Point2D operator * ( const Point2D& p, int c ){

        return Point2D( p.x() * c, p.y() * c );
    };
    //

    //
    friend inline const Point2D operator / ( const Point2D& p, int c ){

        return Point2D( p.x() / c, p.y() / c );
    };
    //

private:
    int p_[2];
};
// ! Point2D

// PointF2D: 2d point with double precision
class PointF2D
{

public:
    PointF2D( void ) { p_[0] = 0.0f; p_[1] = 0.0f; };
    PointF2D( double x, double y ) { p_[0] = x; p_[1] = y; };
    ~PointF2D( void ) {};

    inline double x( void ) const { return p_[0]; };
    inline double y( void ) const { return p_[1]; };
    inline void SetX( double x ) { p_[0] = x; };
    inline void SetY( double y ) { p_[1] = y; };
    inline double* data() { return p_; }

    void operator += ( const PointF2D& p ) { p_[0] += p.x(); p_[1] += p.y(); };
    void operator += ( double c ) { p_[0] += c; p_[1] += c; };
    void operator -= ( const PointF2D& p ) { p_[0] -= p.x(); p_[1] -= p.y(); };
    void operator -= ( double c ) { p_[0] -= c; p_[1] -= c; };
    void operator *= ( double c ) { p_[0] *= c; p_[1] *= c; };
    void operator /= ( double c ) { p_[0] /= c; p_[1] /= c; };

    inline double& operator []( int index ) { return p_[index]; };

    const PointF2D& operator = ( const PointF2D& p ) { p_[0] = p.x(); p_[1] = p.y(); return p; };

    //
    friend inline bool operator == ( const PointF2D& p1, const PointF2D& p2 ){

        return ( ( p1.p_[0] == p2.p_[0] ) && ( p1.p_[1] == p2.p_[1] ) );
    };
    //

    //
    friend inline bool operator != ( const PointF2D& p1, const PointF2D& p2 ){

        return ( ( p1.p_[0] != p2.p_[0] ) || ( p1.p_[1] != p2.p_[1] ) );
    };
    //

    //
    friend inline const PointF2D operator + ( const PointF2D& p1, const PointF2D& p2 ){

        return PointF2D( p1.x() + p2.x(), p1.y() + p2.y() );
    };
    //

    // 
    friend inline const PointF2D operator + ( const PointF2D& p, double c ){

        return PointF2D( p.x() + c, p.y() + c );
    };
    //

    //
    friend inline const PointF2D operator - ( const PointF2D& p1, const PointF2D& p2 ){

        return PointF2D( p1.x() - p2.x(), p1.y() - p2.y() );
    };
    //

    // 
    friend inline const PointF2D operator - ( const PointF2D& p, double c ){

        return PointF2D( p.x() - c, p.y() - c );
    };
    //

    // 
    friend inline const PointF2D operator * ( const PointF2D& p, double c ){

        return PointF2D( p.x() * c, p.y() * c );
    };
    //

    //
    friend inline const PointF2D operator / ( const PointF2D& p, double c ){
    
        return PointF2D( p.x() / c, p.y() / c );
    };
    //

private:
    double p_[2];
};
// ! PointF2D

// Point3D: 3d point with int precision
class Point3D
{

public:
    Point3D( void ) { p_[0] = 0; p_[1] = 0; p_[2] = 0; };
    Point3D( int x, int y, int z ) { p_[0] = x; p_[1] = y; p_[2] = z; };
    ~Point3D( void ) {};

    inline int x( void ) const { return p_[0]; };
    inline int y( void ) const { return p_[1]; };
    inline int z( void ) const { return p_[2]; };
    inline void SetX( int x ) { p_[0] = x; };
    inline void SetY( int y ) { p_[1] = y; };
    inline void SetZ( int z ) { p_[2] = z; };
    inline int* data() { return p_; }

    void operator += ( const Point3D& p ) { p_[0] += p.x(); p_[1] += p.y(); p_[2] += p.z(); };
    void operator += ( int c ) { p_[0] += c; p_[1] += c; p_[2] += c; };
    void operator -= ( const Point3D& p ) { p_[0] -= p.x(); p_[1] -= p.y(); p_[2] -= p.z(); };
    void operator -= ( int c ) { p_[0] -= c; p_[1] -= c; p_[2] -= c; };
    void operator *= ( int c ) { p_[0] *= c; p_[1] *= c; p_[2] *= c; };
    void operator /= ( int c ) { p_[0] /= c; p_[1] /= c; p_[2] /= c; };

    inline int& operator []( int index ) { return p_[index]; };

    const Point3D& operator = ( const Point3D& p ) { p_[0] = p.x(); p_[1] = p.y(); p_[2] = p.z(); return p; };

    //
    friend inline bool operator == ( const Point3D& p1, const Point3D& p2 ){

        return ( ( p1.p_[0] == p2.p_[0] ) && ( p1.p_[1] == p2.p_[1] ) && ( p1.p_[2] == p2.p_[2] ) );
    };
    //

    //
    friend inline bool operator != ( const Point3D& p1, const Point3D& p2 ){

        return ( ( p1.p_[0] != p2.p_[0] ) || ( p1.p_[1] != p2.p_[1] ) || ( p1.p_[2] != p2.p_[2] ) );
    };
    //

    //
    friend inline const Point3D operator + ( const Point3D& p1, const Point3D& p2 ){

        return Point3D( p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z() );
    };
    //

    // 
    friend inline const Point3D operator + ( const Point3D& p, int c ){

        return Point3D( p.x() + c, p.y() + c, p.z() + c );
    };
    //

    //
    friend inline const Point3D operator - ( const Point3D& p1, const Point3D& p2 ){

        return Point3D( p1.x() - p2.x(), p1.y() - p2.y(), p1.z() - p2.z() );
    };
    //

    // 
    friend inline const Point3D operator - ( const Point3D& p, int c ){

        return Point3D( p.x() - c, p.y() - c, p.z() - c );
    };
    //

    // 
    friend inline const Point3D operator * ( const Point3D& p, int c ){

        return Point3D( p.x() * c, p.y() * c, p.z() * c );
    };
    //

    //
    friend inline const Point3D operator / ( const Point3D& p, int c ){

        return Point3D( p.x() / c, p.y() / c, p.z() / c );
    };
    //

private:
    int p_[3];
};
// ! Point3D

// PointF3D: 3d point with double precision
class PointF3D
{

public:
    PointF3D( void ) { p_[0] = 0.0f; p_[1] = 0.0f; p_[2] = 0.0f; };
	PointF3D( double x, double y, double z ) { p_[0] = x; p_[1] = y; p_[2] = z; };
    ~PointF3D( void ) {};

    inline double x( void ) const { return p_[0]; };
    inline double y( void ) const { return p_[1]; };
    inline double z( void ) const { return p_[2]; };
    inline void SetX( double x ) { p_[0] = x; };
    inline void SetY( double y ) { p_[1] = y; };
    inline void SetZ( double z ) { p_[2] = z; };
    inline double* data() { return p_; }

    void operator += ( const PointF3D& p ) { p_[0] += p.x(); p_[1] += p.y(); p_[2] += p.z(); };
    void operator += ( double c ) { p_[0] += c; p_[1] += c; p_[2] += c; };
    void operator -= ( const PointF3D& p ) { p_[0] -= p.x(); p_[1] -= p.y(); p_[2] -= p.z(); };
    void operator -= ( double c ) { p_[0] -= c; p_[1] -= c; p_[2] -= c; };
    void operator *= ( double c ) { p_[0] *= c; p_[1] *= c; p_[2] *= c; };
    void operator /= ( double c ) { p_[0] /= c; p_[1] /= c; p_[2] /= c; };

    inline double& operator []( int index ) { return p_[index]; };

    const PointF3D& operator = ( const PointF3D& p ) { p_[0] = p.x(); p_[1] = p.y(); p_[2] = p.z(); return p; };

    //
    friend inline bool operator == ( const PointF3D& p1, const PointF3D& p2 ){

        return ( ( p1.p_[0] == p2.p_[0] ) && ( p1.p_[1] == p2.p_[1] ) && ( p1.p_[2] == p2.p_[2] ) );
    };
    //

    //
    friend inline bool operator != ( const PointF3D& p1, const PointF3D& p2 ){

        return ( ( p1.p_[0] != p2.p_[0] ) || ( p1.p_[1] != p2.p_[1] ) || ( p1.p_[2] != p2.p_[2] ) );
    };
    //

    //
    friend inline const PointF3D operator + ( const PointF3D& p1, const PointF3D& p2 ){

        return PointF3D( p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z() );
    };
    //

    // 
    friend inline const PointF3D operator + ( const PointF3D& p, double c ){

        return PointF3D( p.x() + c, p.y() + c, p.z() + c );
    };
    //

    //
    friend inline const PointF3D operator - ( const PointF3D& p1, const PointF3D& p2 ){

        return PointF3D( p1.x() - p2.x(), p1.y() - p2.y(), p1.z() - p2.z() );
    };
    //

    // 
    friend inline const PointF3D operator - ( const PointF3D& p, double c ){

        return PointF3D( p.x() - c, p.y() - c, p.z() - c );
    };
    //

    // 
    friend inline const PointF3D operator * ( const PointF3D& p, double c ){

        return PointF3D( p.x() * c, p.y() * c, p.z() * c );
    };
    //

    friend inline double operator * ( const PointF3D& p1, const PointF3D& p2 ){

        return p1.x() * p2.x() + p1.y() * p2.y() + p1.z() * p2.z();
    };

    //
    friend inline const PointF3D operator / ( const PointF3D& p, double c ){

        return PointF3D( p.x() / c, p.y() / c, p.z() / c );
    };
    //
	static inline double VectorMag(const VectorF3D &a){
		return sqrt(a.x() * a.x() + a.y() * a.y() + a.z() * a.z());
	}

	static inline VectorF3D CrossProductV(const VectorF3D &a, const VectorF3D &b){
		return VectorF3D(
			a.y() * b.z() - a.z() * b.y(),
			a.z() * b.x() - a.x() * b.z(),
			a.x() * b.y() - a.y() * b.x()
			);
	}

	static inline void NormalizeV(VectorF3D &v){
		double magSq = VectorMag(v);

		if (magSq > 0)
		{
			double oneOverMag = 1.0 / magSq;
			v.SetX(v.x() * oneOverMag);
			v.SetY(v.y() * oneOverMag);
			v.SetZ(v.z() * oneOverMag);
		}
	}

private:
    double p_[3];
};
// ! PointF3D

//
#endif // !defined( EA_C0513EFB_DC3B_4013_B468_0D717CCFD97C__INCLUDED_ )
