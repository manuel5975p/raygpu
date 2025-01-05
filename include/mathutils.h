#ifndef MATHUTILS_H
#define MATHUTILS_H
/*   Copyright (c) 2015-2024 Ramon Santamaria (@raysan5)
 *
 *   This software is provided "as-is", without any express or implied warranty. In no event
 *   will the authors be held liable for any damages arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose, including commercial
 *   applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not claim that you
 *     wrote the original software. If you use this software in a product, an acknowledgment
 *     in the product documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *     as being the original software.
 *
 *     3. This notice may not be removed or altered from any source distribution.
 *
 **********************************************************************************************
  
 * This file contains altered source of raymath.h by Ramon Santamaria (@raysan5)
 */
#include <macros_and_constants.h>
#ifdef __cplusplus
#include <cmath>
#include <iostream>
using std::sqrt;
using std::tan;
#else
#include <math.h>
#endif
#define MAPI static inline
#ifndef M_PI
#define M_PI 3.1415926
#endif

typedef struct Vector4{
    float x,y,z,w;
    #ifdef __cplusplus
    Vector4& operator+=(const Vector4& o)noexcept{x+=o.x;y+=o.y;z+=o.z;w+=o.w;return *this;}
    Vector4& operator-=(const Vector4& o)noexcept{x-=o.x;y-=o.y;z-=o.z;w-=o.w;return *this;}
    Vector4& operator*=(const Vector4& o)noexcept{x*=o.x;y*=o.y;z*=o.z;w*=o.w;return *this;}
    Vector4& operator/=(const Vector4& o)noexcept{x/=o.x;y/=o.y;z/=o.z;w/=o.w;return *this;}
    Vector4& operator*=(float o)noexcept{x*=o;y*=o;z*=o;w*=o;return *this;}
    Vector4& operator/=(float o)noexcept{x/=o;y/=o;z/=o;w/=o;return *this;}
    Vector4  operator+ (const Vector4& o)const noexcept{Vector4 ret(*this);ret += o;return ret;}
    Vector4  operator- (const Vector4& o)const noexcept{Vector4 ret(*this);ret -= o;return ret;}
    Vector4  operator* (const Vector4& o)const noexcept{Vector4 ret(*this);ret *= o;return ret;}
    Vector4  operator/ (const Vector4& o)const noexcept{Vector4 ret(*this);ret /= o;return ret;}
    Vector4  operator* (float o)const noexcept{Vector4 ret(*this);ret *=o;return ret;}
    Vector4  operator/ (float o)const noexcept{Vector4 ret(*this);ret /=o;return ret;}
    #endif
}Vector4;

typedef struct Vector3{
    float x,y,z;
    #ifdef __cplusplus
    Vector3& operator+=(const Vector3& o)noexcept{x+=o.x;y+=o.y;z+=o.z;return *this;}
    Vector3& operator-=(const Vector3& o)noexcept{x-=o.x;y-=o.y;z-=o.z;return *this;}
    Vector3& operator*=(const Vector3& o)noexcept{x*=o.x;y*=o.y;z*=o.z;return *this;}
    Vector3& operator/=(const Vector3& o)noexcept{x/=o.x;y/=o.y;z/=o.z;return *this;}
    Vector3& operator*=(float o)noexcept{x*=o;y*=o;z*=o;return *this;}
    Vector3& operator/=(float o)noexcept{x/=o;y/=o;z/=o;return *this;}
    Vector3  operator+ (const Vector3& o)const noexcept{Vector3 ret(*this);ret += o;return ret;}
    Vector3  operator- (const Vector3& o)const noexcept{Vector3 ret(*this);ret -= o;return ret;}
    Vector3  operator* (const Vector3& o)const noexcept{Vector3 ret(*this);ret *= o;return ret;}
    Vector3  operator/ (const Vector3& o)const noexcept{Vector3 ret(*this);ret /= o;return ret;}
    Vector3  operator* (float o)const noexcept{Vector3 ret(*this);ret *=o;return ret;}
    Vector3  operator/ (float o)const noexcept{Vector3 ret(*this);ret /=o;return ret;}
    #endif
}Vector3;

typedef struct Vector2{
    float x,y;
    #ifdef __cplusplus
    Vector2& operator+=(const Vector2& o)noexcept{x+=o.x;y+=o.y;return *this;}
    Vector2& operator-=(const Vector2& o)noexcept{x-=o.x;y-=o.y;return *this;}
    Vector2& operator*=(const Vector2& o)noexcept{x*=o.x;y*=o.y;return *this;}
    Vector2& operator/=(const Vector2& o)noexcept{x/=o.x;y/=o.y;return *this;}
    Vector2& operator*=(float o)noexcept{x*=o;y*=o;return *this;}
    Vector2& operator/=(float o)noexcept{x/=o;y/=o;return *this;}
    Vector2  operator+ (const Vector2& o)const noexcept{Vector2 ret(*this);ret += o;return ret;}
    Vector2  operator- (const Vector2& o)const noexcept{Vector2 ret(*this);ret -= o;return ret;}
    Vector2  operator* (const Vector2& o)const noexcept{Vector2 ret(*this);ret *= o;return ret;}
    Vector2  operator/ (const Vector2& o)const noexcept{Vector2 ret(*this);ret /= o;return ret;}
    Vector2  operator* (float o)const noexcept{Vector2 ret(*this);ret *=o;return ret;}
    Vector2  operator/ (float o)const noexcept{Vector2 ret(*this);ret /=o;return ret;}
    #endif
}Vector2;

MAPI Vector2 Vector2Normalize(Vector2 vec){
    float one_by_nrm = 1.0f / sqrt(vec.x * vec.x + vec.y * vec.y);
    vec.x *= one_by_nrm;
    vec.y *= one_by_nrm;
    return vec;
}
MAPI Vector3 Vector3Normalize(Vector3 vec){
    float one_by_nrm = 1.0f / sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    vec.x *= one_by_nrm;
    vec.y *= one_by_nrm;
    vec.z *= one_by_nrm;
    return vec;
}
MAPI Vector4 Vector4Normalize(Vector4 vec){
    float one_by_nrm = 1.0f / sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
    vec.x *= one_by_nrm;
    vec.y *= one_by_nrm;
    vec.z *= one_by_nrm;
    vec.w *= one_by_nrm;
    return vec;
}

typedef struct Matrix{
    float data[16];
    #ifdef __cplusplus
    Matrix operator*(const Matrix& o)const noexcept{
        Matrix ret zeroinit;
        for(int i = 0;i < 4;i++){
            for(int j = 0;j < 4;j++){
                for(int k = 0;k < 4;k++){
                    ret.data[i + j * 4] += data[i + k * 4] * o.data[k + j * 4];
                }
            }
        }
        return ret;
    }
    #endif// __cplusplus
} Matrix;

//typedef struct Quaternion{
//    float x,y,z,w;
//}Quaternion;
typedef Vector4 Quaternion;

typedef struct Transform {
    Vector3 translation;    // Translation
    Quaternion rotation;    // Rotation
    Vector3 scale;          // Scale
} Transform;

MAPI Matrix MatrixIdentity(void){
    Matrix ret zeroinit;
    ret.data[0] = 1;
    ret.data[5] = 1;
    ret.data[10] = 1;
    ret.data[15] = 1;
    return ret;
}


MAPI Matrix MatrixMultiply(Matrix A, Matrix B){
    Matrix ret zeroinit;
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            for(int k = 0;k < 4;k++){
                ret.data[i + j * 4] += A.data[i + k * 4] * B.data[k + j * 4];
            }
        }
    }
    return ret;
}
MAPI Matrix MatrixMultiplySwap(Matrix A, Matrix B){
   return MatrixMultiply(B, A);
}
MAPI Vector3 Vector3Subtract(Vector3 v1, Vector3 v2){
    Vector3 result = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return result;
}
typedef struct Camera2D {
    Vector2 offset;         // Camera offset (displacement from target)
    Vector2 target;         // Camera target (rotation and zoom origin)
    float rotation;         // Camera rotation in degrees
    float zoom;             // Camera zoom (scaling), should be 1.0f by default
} Camera2D;

typedef struct Camera3D {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fovy;
} Camera3D;
typedef Camera3D Camera;    // Camera type fallback, defaults to Camera3D

// https://github.com/raysan5/raylib/blob/master/src/raymath.h
MAPI Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up){
    Matrix result;

    float length = 0.0f;
    float ilength = 0.0f;

    Vector3 vz = Vector3Subtract(eye, target);

    // Vector3Normalize(vz)
    Vector3 v = vz;
    length = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vz.x *= ilength;
    vz.y *= ilength;
    vz.z *= ilength;

    // Vector3CrossProduct(up, vz)
    Vector3 vx = { up.y*vz.z - up.z*vz.y, up.z*vz.x - up.x*vz.z, up.x*vz.y - up.y*vz.x };

    // Vector3Normalize(x)
    v = vx;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f / length;
    vx.x *= ilength;
    vx.y *= ilength;
    vx.z *= ilength;

    //Cross Product
    Vector3 vy = { 
        vz.y*vx.z - vz.z*vx.y,
        vz.z*vx.x - vz.x*vx.z,
        vz.x*vx.y - vz.y*vx.x
    };
    //#define idx_scramble(i) ((i % 4) * 4 + (i / 4))
    #define idx_map(i, j) (i + j * 4)
    #define idx_scramble(i) (i)
    result.data[idx_scramble(0 )] = vx.x;
    result.data[idx_scramble(1 )] = vy.x;
    result.data[idx_scramble(2 )] = vz.x;
    result.data[idx_scramble(3 )] = 0.0f;
    result.data[idx_scramble(4 )] = vx.y;
    result.data[idx_scramble(5 )] = vy.y;
    result.data[idx_scramble(6 )] = vz.y;
    result.data[idx_scramble(7 )] = 0.0f;
    result.data[idx_scramble(8 )] = vx.z;
    result.data[idx_scramble(9 )] = vy.z;
    result.data[idx_scramble(10)] = vz.z;
    result.data[idx_scramble(11)] = 0.0f;
    result.data[idx_scramble(12)] = -(vx.x * eye.x + vx.y * eye.y + vx.z * eye.z);   // Vector3DotProduct(vx, eye)
    result.data[idx_scramble(13)] = -(vy.x * eye.x + vy.y * eye.y + vy.z * eye.z);   // Vector3DotProduct(vy, eye)
    result.data[idx_scramble(14)] = -(vz.x * eye.x + vz.y * eye.y + vz.z * eye.z);   // Vector3DotProduct(vz, eye)
    result.data[idx_scramble(15)] = 1.0f;

    return result;
}
// https://github.com/raysan5/raylib/blob/master/src/raymath.h
/**
    @brief Perspective (radians arguments)
*/
MAPI Matrix MatrixPerspective(double fovY, double aspect, double nearPlane, double farPlane){
    Matrix result zeroinit;

    double top = nearPlane * tan(fovY * 0.5);
    double bottom = -top;
    double right = top*aspect;
    double left = -right;

    // MatrixFrustum(-right, right, -top, top, near, far);
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(farPlane - nearPlane);

    result.data[idx_scramble(0 )] = ((float)nearPlane*2.0f)/rl;
    result.data[idx_scramble(5 )] = ((float)nearPlane*2.0f)/tb;
    result.data[idx_scramble(8 )] = ((float)right + (float)left)/rl;
    result.data[idx_scramble(9 )] = ((float)top + (float)bottom)/tb;
    result.data[idx_scramble(10)] = -((float)farPlane + (float)nearPlane)/fn;
    result.data[idx_scramble(11)] = -1.0f;
    result.data[idx_scramble(14)] = -((float)farPlane*(float)nearPlane*2.0f)/fn;

    return result;
}
MAPI Matrix ScreenMatrix(int width, int height) {
    Matrix ret = MatrixIdentity();
    ret.data[idx_map(0, 3)] = -1.0f; // X translation
    ret.data[idx_map(1, 3)] = 1.0f;  // Y translation (move to top)
    ret.data[0] = 2.0f / ((float)width); // X scale
    ret.data[5] = -2.0f / ((float)height); // Y scale (flip Y-axis)
    ret.data[10] = 1; // Z scale
    ret.data[15] = 1; // W scale
    return ret;
}

MAPI Matrix ScreenMatrixBottomY(int width, int height) {
    Matrix ret = MatrixIdentity();
    ret.data[idx_map(0, 3)] = -1.0f;
    ret.data[idx_map(1, 3)] = -1.0f;
    ret.data[0] = 2.0f / ((float)width);
    ret.data[5] = 2.0f / ((float)height);
    ret.data[10] = 1;
    ret.data[15] = 1;
    return ret;
}

MAPI Matrix MatrixTranspose(Matrix mat){
    Matrix ret zeroinit;
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            ret.data[j * 4 + i] = mat.data[i * 4 + j];
        }
    }
    return ret;
}
MAPI Vector4 Vector4Scale(Vector4 x, float a){
    return CLITERAL(Vector4){a * x.x, a * x.y, a * x.z, a * x.w};
}
MAPI Vector3 Vector3Scale(Vector3 x, float a){
    return CLITERAL(Vector3){a * x.x, a * x.y, a * x.z};
}
MAPI Vector2 Vector2Scale(Vector2 x, float a){
    return CLITERAL(Vector2){a * x.x, a * x.y};
}
MAPI Vector4 Vector4Negate(Vector4 x){
    return CLITERAL(Vector4){-x.x, -x.y, -x.z, -x.w};
}
MAPI Vector3 Vector3Negate(Vector3 x){
    return CLITERAL(Vector3){-x.x, -x.y, -x.z};
}
MAPI Vector2 Vector2Negate(Vector2 x){
    return CLITERAL(Vector2){-x.x, -x.y};
}
MAPI int FloatEquals(float x, float y){
    #if !defined(EPSILON)
        #define EPSILON 0.000001f
    #endif

    int result = (fabsf(x - y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))));

    return result;
}

MAPI Matrix QuaternionToMatrix(Quaternion q)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    float a2 = q.x * q.x;
    float b2 = q.y * q.y;
    float c2 = q.z * q.z;
    float ac = q.x * q.z;
    float ab = q.x * q.y;
    float bc = q.y * q.z;
    float ad = q.w * q.x;
    float bd = q.w * q.y;
    float cd = q.w * q.z;

    result.data[0] = 1 - 2*(b2 + c2);
    result.data[1] = 2*(ab + cd);
    result.data[2] = 2*(ac - bd);

    result.data[4] = 2*(ab - cd);
    result.data[5] = 1 - 2*(a2 + c2);
    result.data[6] = 2*(bc + ad);

    result.data[8] = 2*(ac + bd);
    result.data[9] = 2*(bc - ad);
    result.data[10] = 1 - 2*(a2 + b2);

    return result;
}



// Get a quaternion for a given rotation matrix
MAPI Quaternion QuaternionFromMatrix(Matrix mat)
{
    Quaternion result zeroinit;

    float fourWSquaredMinus1 = mat.data[0 ] + mat.data[5] + mat.data[10];
    float fourXSquaredMinus1 = mat.data[0 ] - mat.data[5] - mat.data[10];
    float fourYSquaredMinus1 = mat.data[5 ] - mat.data[0] - mat.data[10];
    float fourZSquaredMinus1 = mat.data[10] - mat.data[0] - mat.data[5 ];

    int biggestIndex = 0;
    float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
    if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourXSquaredMinus1;
        biggestIndex = 1;
    }

    if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourYSquaredMinus1;
        biggestIndex = 2;
    }

    if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourZSquaredMinus1;
        biggestIndex = 3;
    }

    float biggestVal = sqrtf(fourBiggestSquaredMinus1 + 1.0f)*0.5f;
    float mult = 0.25f/biggestVal;

    switch (biggestIndex)
    {
        case 0:
            result.w = biggestVal;
            result.x = (mat.data[6] - mat.data[9])*mult;
            result.y = (mat.data[8] - mat.data[2])*mult;
            result.z = (mat.data[1] - mat.data[4])*mult;
            break;
        case 1:
            result.x = biggestVal;
            result.w = (mat.data[6] - mat.data[9])*mult;
            result.y = (mat.data[1] + mat.data[4])*mult;
            result.z = (mat.data[8] + mat.data[2])*mult;
            break;
        case 2:
            result.y = biggestVal;
            result.w = (mat.data[8] - mat.data[2])*mult;
            result.x = (mat.data[1] + mat.data[4])*mult;
            result.z = (mat.data[6] + mat.data[9])*mult;
            break;
        case 3:
            result.z = biggestVal;
            result.w = (mat.data[1] - mat.data[4])*mult;
            result.x = (mat.data[8] + mat.data[2])*mult;
            result.y = (mat.data[6] + mat.data[9])*mult;
            break;
    }

    return result;
}
MAPI Vector4 Vector4Zero(void)
{
    Vector4 result = { 0.0f, 0.0f, 0.0f, 0.0f };
    return result;
}

MAPI Vector4 Vector4One(void)
{
    Vector4 result = { 1.0f, 1.0f, 1.0f, 1.0f };
    return result;
}

MAPI Vector4 Vector4Add(Vector4 v1, Vector4 v2)
{
    Vector4 result = {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z,
        v1.w + v2.w
    };
    return result;
}

MAPI Vector4 Vector4AddValue(Vector4 v, float add)
{
    Vector4 result = {
        v.x + add,
        v.y + add,
        v.z + add,
        v.w + add
    };
    return result;
}

MAPI Vector4 Vector4Subtract(Vector4 v1, Vector4 v2)
{
    Vector4 result = {
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z,
        v1.w - v2.w
    };
    return result;
}

MAPI Vector4 Vector4SubtractValue(Vector4 v, float add)
{
    Vector4 result = {
        v.x - add,
        v.y - add,
        v.z - add,
        v.w - add
    };
    return result;
}

MAPI float Vector4Length(Vector4 v)
{
    float result = sqrtf((v.x*v.x) + (v.y*v.y) + (v.z*v.z) + (v.w*v.w));
    return result;
}

MAPI float Vector4LengthSqr(Vector4 v)
{
    float result = (v.x*v.x) + (v.y*v.y) + (v.z*v.z) + (v.w*v.w);
    return result;
}

MAPI float Vector4DotProduct(Vector4 v1, Vector4 v2)
{
    float result = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w);
    return result;
}

// Calculate distance between two vectors
MAPI float Vector4Distance(Vector4 v1, Vector4 v2)
{
    float result = sqrtf(
        (v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y) +
        (v1.z - v2.z)*(v1.z - v2.z) + (v1.w - v2.w)*(v1.w - v2.w));
    return result;
}

// Calculate square distance between two vectors
MAPI float Vector4DistanceSqr(Vector4 v1, Vector4 v2)
{
    float result =
        (v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y) +
        (v1.z - v2.z)*(v1.z - v2.z) + (v1.w - v2.w)*(v1.w - v2.w);

    return result;
}


// Multiply vector by vector
MAPI Vector4 Vector4Multiply(Vector4 v1, Vector4 v2)
{
    Vector4 result = { v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w };
    return result;
}


// Divide vector by vector
MAPI Vector4 Vector4Divide(Vector4 v1, Vector4 v2)
{
    Vector4 result = { v1.x/v2.x, v1.y/v2.y, v1.z/v2.z, v1.w/v2.w };
    return result;
}


// Get min value for each pair of components
MAPI Vector4 Vector4Min(Vector4 v1, Vector4 v2)
{
    Vector4 result zeroinit;

    result.x = fminf(v1.x, v2.x);
    result.y = fminf(v1.y, v2.y);
    result.z = fminf(v1.z, v2.z);
    result.w = fminf(v1.w, v2.w);

    return result;
}

// Get max value for each pair of components
MAPI Vector4 Vector4Max(Vector4 v1, Vector4 v2)
{
    Vector4 result zeroinit;

    result.x = fmaxf(v1.x, v2.x);
    result.y = fmaxf(v1.y, v2.y);
    result.z = fmaxf(v1.z, v2.z);
    result.w = fmaxf(v1.w, v2.w);

    return result;
}

// Calculate linear interpolation between two vectors
MAPI Vector4 Vector4Lerp(Vector4 v1, Vector4 v2, float amount)
{
    Vector4 result zeroinit;

    result.x = v1.x + amount*(v2.x - v1.x);
    result.y = v1.y + amount*(v2.y - v1.y);
    result.z = v1.z + amount*(v2.z - v1.z);
    result.w = v1.w + amount*(v2.w - v1.w);

    return result;
}

// Move Vector towards target
MAPI Vector4 Vector4MoveTowards(Vector4 v, Vector4 target, float maxDistance)
{
    Vector4 result zeroinit;

    float dx = target.x - v.x;
    float dy = target.y - v.y;
    float dz = target.z - v.z;
    float dw = target.w - v.w;
    float value = (dx*dx) + (dy*dy) + (dz*dz) + (dw*dw);

    if ((value == 0) || ((maxDistance >= 0) && (value <= maxDistance*maxDistance))) return target;

    float dist = sqrtf(value);

    result.x = v.x + dx/dist*maxDistance;
    result.y = v.y + dy/dist*maxDistance;
    result.z = v.z + dz/dist*maxDistance;
    result.w = v.w + dw/dist*maxDistance;

    return result;
}

// Invert the given vector
MAPI Vector4 Vector4Invert(Vector4 v)
{
    Vector4 result = { 1.0f/v.x, 1.0f/v.y, 1.0f/v.z, 1.0f/v.w };
    return result;
}

// Check whether two given vectors are almost equal
MAPI int Vector4Equals(Vector4 p, Vector4 q)
{
#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    int result = ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                  ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
                  ((fabsf(p.z - q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) &&
                  ((fabsf(p.w - q.w)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w)))));
    return result;
}
//Calculate the projection of the vector v1 on to v2
MAPI Vector3 Vector3Project(Vector3 v1, Vector3 v2)
{
    Vector3 result zeroinit;

    float v1dv2 = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
    float v2dv2 = (v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);

    float mag = v1dv2/v2dv2;

    result.x = v2.x*mag;
    result.y = v2.y*mag;
    result.z = v2.z*mag;

    return result;
}

//Calculate the rejection of the vector v1 on to v2
MAPI Vector3 Vector3Reject(Vector3 v1, Vector3 v2)
{
    Vector3 result zeroinit;

    float v1dv2 = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
    float v2dv2 = (v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);

    float mag = v1dv2/v2dv2;

    result.x = v1.x - (v2.x*mag);
    result.y = v1.y - (v2.y*mag);
    result.z = v1.z - (v2.z*mag);

    return result;
}

// Orthonormalize provided vectors
// Makes vectors normalized and orthogonal to each other
// Gram-Schmidt function implementation
MAPI void Vector3OrthoNormalize(Vector3 *v1, Vector3 *v2)
{
    float length = 0.0f;
    float ilength = 0.0f;

    // Vector3Normalize(*v1);
    Vector3 v = *v1;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    v1->x *= ilength;
    v1->y *= ilength;
    v1->z *= ilength;

    // Vector3CrossProduct(*v1, *v2)
    Vector3 vn1 = { v1->y*v2->z - v1->z*v2->y, v1->z*v2->x - v1->x*v2->z, v1->x*v2->y - v1->y*v2->x };

    // Vector3Normalize(vn1);
    v = vn1;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vn1.x *= ilength;
    vn1.y *= ilength;
    vn1.z *= ilength;

    // Vector3CrossProduct(vn1, *v1)
    Vector3 vn2 = { vn1.y*v1->z - vn1.z*v1->y, vn1.z*v1->x - vn1.x*v1->z, vn1.x*v1->y - vn1.y*v1->x };

    *v2 = vn2;
}

// Transforms a Vector3 by a given Matrix
MAPI Vector3 Vector3Transform(Vector3 v, Matrix mat)
{
    Vector3 result zeroinit;

    float x = v.x;
    float y = v.y;
    float z = v.z;

    result.x = mat.data[0]*x + mat.data[4]*y + mat.data[8]*z + mat.data[12];
    result.y = mat.data[0]*x + mat.data[5]*y + mat.data[9]*z + mat.data[13];
    result.z = mat.data[0]*x + mat.data[6]*y + mat.data[10]*z + mat.data[14];

    return result;
}

// Transform a vector by quaternion rotation
MAPI Vector3 Vector3RotateByQuaternion(Vector3 v, Quaternion q)
{
    Vector3 result zeroinit;

    result.x = v.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + v.y*(2*q.x*q.y - 2*q.w*q.z) + v.z*(2*q.x*q.z + 2*q.w*q.y);
    result.y = v.x*(2*q.w*q.z + 2*q.x*q.y) + v.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + v.z*(-2*q.w*q.x + 2*q.y*q.z);
    result.z = v.x*(-2*q.w*q.y + 2*q.x*q.z) + v.y*(2*q.w*q.x + 2*q.y*q.z)+ v.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);

    return result;
}

// Rotates a vector around an axis
MAPI Vector3 Vector3RotateByAxisAngle(Vector3 v, Vector3 axis, float angle)
{
    // Using Euler-Rodrigues Formula
    // Ref.: https://en.wikipedia.org/w/index.php?title=Euler%E2%80%93Rodrigues_formula

    Vector3 result = v;

    // Vector3Normalize(axis);
    float length = sqrtf(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
    if (length == 0.0f) length = 1.0f;
    float ilength = 1.0f/length;
    axis.x *= ilength;
    axis.y *= ilength;
    axis.z *= ilength;

    angle /= 2.0f;
    float a = sinf(angle);
    float b = axis.x*a;
    float c = axis.y*a;
    float d = axis.z*a;
    a = cosf(angle);
    Vector3 w = { b, c, d };

    // Vector3CrossProduct(w, v)
    Vector3 wv = { w.y*v.z - w.z*v.y, w.z*v.x - w.x*v.z, w.x*v.y - w.y*v.x };

    // Vector3CrossProduct(w, wv)
    Vector3 wwv = { w.y*wv.z - w.z*wv.y, w.z*wv.x - w.x*wv.z, w.x*wv.y - w.y*wv.x };

    // Vector3Scale(wv, 2*a)
    a *= 2;
    wv.x *= a;
    wv.y *= a;
    wv.z *= a;

    // Vector3Scale(wwv, 2)
    wwv.x *= 2;
    wwv.y *= 2;
    wwv.z *= 2;

    result.x += wv.x;
    result.y += wv.y;
    result.z += wv.z;

    result.x += wwv.x;
    result.y += wwv.y;
    result.z += wwv.z;

    return result;
}

// Move Vector towards target
MAPI Vector3 Vector3MoveTowards(Vector3 v, Vector3 target, float maxDistance)
{
    Vector3 result zeroinit;

    float dx = target.x - v.x;
    float dy = target.y - v.y;
    float dz = target.z - v.z;
    float value = (dx*dx) + (dy*dy) + (dz*dz);

    if ((value == 0) || ((maxDistance >= 0) && (value <= maxDistance*maxDistance))) return target;

    float dist = sqrtf(value);

    result.x = v.x + dx/dist*maxDistance;
    result.y = v.y + dy/dist*maxDistance;
    result.z = v.z + dz/dist*maxDistance;

    return result;
}

// Calculate linear interpolation between two vectors
MAPI Vector3 Vector3Lerp(Vector3 v1, Vector3 v2, float amount)
{
    Vector3 result zeroinit;

    result.x = v1.x + amount*(v2.x - v1.x);
    result.y = v1.y + amount*(v2.y - v1.y);
    result.z = v1.z + amount*(v2.z - v1.z);

    return result;
}
MAPI Vector3 Vector3Add(Vector3 v1, Vector3 v2)
{
    Vector3 result = {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z
    };
    return result;
}
MAPI Vector3 Vector3Multiply(Vector3 v1, Vector3 v2)
{
    Vector3 result = { v1.x*v2.x, v1.y*v2.y, v1.z*v2.z };
    return result;
}
MAPI Vector3 Vector3Divide(Vector3 v1, Vector3 v2)
{
    Vector3 result = { v1.x/v2.x, v1.y/v2.y, v1.z/v2.z };

    return result;
}
// Calculate cubic hermite interpolation between two vectors and their tangents
// as described in the GLTF 2.0 specification: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#interpolation-cubic
MAPI Vector3 Vector3CubicHermite(Vector3 v1, Vector3 tangent1, Vector3 v2, Vector3 tangent2, float amount)
{
    Vector3 result zeroinit;

    float amountPow2 = amount*amount;
    float amountPow3 = amount*amount*amount;

    result.x = (2*amountPow3 - 3*amountPow2 + 1)*v1.x + (amountPow3 - 2*amountPow2 + amount)*tangent1.x + (-2*amountPow3 + 3*amountPow2)*v2.x + (amountPow3 - amountPow2)*tangent2.x;
    result.y = (2*amountPow3 - 3*amountPow2 + 1)*v1.y + (amountPow3 - 2*amountPow2 + amount)*tangent1.y + (-2*amountPow3 + 3*amountPow2)*v2.y + (amountPow3 - amountPow2)*tangent2.y;
    result.z = (2*amountPow3 - 3*amountPow2 + 1)*v1.z + (amountPow3 - 2*amountPow2 + amount)*tangent1.z + (-2*amountPow3 + 3*amountPow2)*v2.z + (amountPow3 - amountPow2)*tangent2.z;

    return result;
}

// Calculate reflected vector to normal
MAPI Vector3 Vector3Reflect(Vector3 v, Vector3 normal)
{
    Vector3 result zeroinit;

    // I is the original vector
    // N is the normal of the incident plane
    // R = I - (2*N*(DotProduct[I, N]))

    float dotProduct = (v.x*normal.x + v.y*normal.y + v.z*normal.z);

    result.x = v.x - (2.0f*normal.x)*dotProduct;
    result.y = v.y - (2.0f*normal.y)*dotProduct;
    result.z = v.z - (2.0f*normal.z)*dotProduct;

    return result;
}

// Get min value for each pair of components
MAPI Vector3 Vector3Min(Vector3 v1, Vector3 v2)
{
    Vector3 result zeroinit;

    result.x = fminf(v1.x, v2.x);
    result.y = fminf(v1.y, v2.y);
    result.z = fminf(v1.z, v2.z);

    return result;
}

// Get max value for each pair of components
MAPI Vector3 Vector3Max(Vector3 v1, Vector3 v2)
{
    Vector3 result zeroinit;

    result.x = fmaxf(v1.x, v2.x);
    result.y = fmaxf(v1.y, v2.y);
    result.z = fmaxf(v1.z, v2.z);

    return result;
}

// Compute barycenter coordinates (u, v, w) for point p with respect to triangle (a, b, c)
// NOTE: Assumes P is on the plane of the triangle
MAPI Vector3 Vector3Barycenter(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
{
    Vector3 result zeroinit;

    Vector3 v0 = { b.x - a.x, b.y - a.y, b.z - a.z };   // Vector3Subtract(b, a)
    Vector3 v1 = { c.x - a.x, c.y - a.y, c.z - a.z };   // Vector3Subtract(c, a)
    Vector3 v2 = { p.x - a.x, p.y - a.y, p.z - a.z };   // Vector3Subtract(p, a)
    float d00 = (v0.x*v0.x + v0.y*v0.y + v0.z*v0.z);    // Vector3DotProduct(v0, v0)
    float d01 = (v0.x*v1.x + v0.y*v1.y + v0.z*v1.z);    // Vector3DotProduct(v0, v1)
    float d11 = (v1.x*v1.x + v1.y*v1.y + v1.z*v1.z);    // Vector3DotProduct(v1, v1)
    float d20 = (v2.x*v0.x + v2.y*v0.y + v2.z*v0.z);    // Vector3DotProduct(v2, v0)
    float d21 = (v2.x*v1.x + v2.y*v1.y + v2.z*v1.z);    // Vector3DotProduct(v2, v1)

    float denom = d00*d11 - d01*d01;

    result.y = (d11*d20 - d01*d21)/denom;
    result.z = (d00*d21 - d01*d20)/denom;
    result.x = 1.0f - (result.z + result.y);

    return result;
}

// Projects a Vector3 from screen space into object space
// NOTE: We are avoiding calling other raymath functions despite available
MAPI Vector3 Vector3Unproject(Vector3 source, Matrix projection, Matrix view)
{
    Vector3 result zeroinit;

    // Calculate unprojected matrix (multiply view matrix by projection matrix) and invert it
    Matrix matViewProj = MatrixMultiplySwap(view, projection);

    // Calculate inverted matrix -> MatrixInvert(matViewProj);
    // Cache the matrix values (speed optimization)
    float a00 = matViewProj.data[0], a01 = matViewProj .data[1], a02 = matViewProj.data[2], a03 = matViewProj.data[3];
    float a10 = matViewProj.data[4], a11 = matViewProj .data[5], a12 = matViewProj.data[6], a13 = matViewProj.data[7];
    float a20 = matViewProj.data[8], a21 = matViewProj .data[9], a22 = matViewProj.data[10], a23 = matViewProj.data[11];
    float a30 = matViewProj.data[12], a31 = matViewProj.data[13], a32 = matViewProj.data[14], a33 = matViewProj.data[15];

    float b00 = a00*a11 - a01*a10;
    float b01 = a00*a12 - a02*a10;
    float b02 = a00*a13 - a03*a10;
    float b03 = a01*a12 - a02*a11;
    float b04 = a01*a13 - a03*a11;
    float b05 = a02*a13 - a03*a12;
    float b06 = a20*a31 - a21*a30;
    float b07 = a20*a32 - a22*a30;
    float b08 = a20*a33 - a23*a30;
    float b09 = a21*a32 - a22*a31;
    float b10 = a21*a33 - a23*a31;
    float b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    float invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    Matrix matViewProjInv = {
        (a11*b11 - a12*b10 + a13*b09)*invDet,
        (-a01*b11 + a02*b10 - a03*b09)*invDet,
        (a31*b05 - a32*b04 + a33*b03)*invDet,
        (-a21*b05 + a22*b04 - a23*b03)*invDet,
        (-a10*b11 + a12*b08 - a13*b07)*invDet,
        (a00*b11 - a02*b08 + a03*b07)*invDet,
        (-a30*b05 + a32*b02 - a33*b01)*invDet,
        (a20*b05 - a22*b02 + a23*b01)*invDet,
        (a10*b10 - a11*b08 + a13*b06)*invDet,
        (-a00*b10 + a01*b08 - a03*b06)*invDet,
        (a30*b04 - a31*b02 + a33*b00)*invDet,
        (-a20*b04 + a21*b02 - a23*b00)*invDet,
        (-a10*b09 + a11*b07 - a12*b06)*invDet,
        (a00*b09 - a01*b07 + a02*b06)*invDet,
        (-a30*b03 + a31*b01 - a32*b00)*invDet,
        (a20*b03 - a21*b01 + a22*b00)*invDet };

    // Create quaternion from source point
    Quaternion quat = { source.x, source.y, source.z, 1.0f };

    // Multiply quat point by unprojecte matrix
    Quaternion qtransformed = {     // QuaternionTransform(quat, matViewProjInv)
        matViewProjInv.data[0]*quat.x + matViewProjInv.data[4]*quat.y + matViewProjInv.data[8]*quat.z + matViewProjInv.data[12]*quat.w,
        matViewProjInv.data[1]*quat.x + matViewProjInv.data[5]*quat.y + matViewProjInv.data[9]*quat.z + matViewProjInv.data[13]*quat.w,
        matViewProjInv.data[2]*quat.x + matViewProjInv.data[6]*quat.y + matViewProjInv.data[10]*quat.z + matViewProjInv.data[14]*quat.w,
        matViewProjInv.data[3]*quat.x + matViewProjInv.data[7]*quat.y + matViewProjInv.data[11]*quat.z + matViewProjInv.data[15]*quat.w };

    // Normalized world points in vectors
    result.x = qtransformed.x/qtransformed.w;
    result.y = qtransformed.y/qtransformed.w;
    result.z = qtransformed.z/qtransformed.w;

    return result;
}

// Invert the given vector
MAPI Vector3 Vector3Invert(Vector3 v)
{
    Vector3 result = { 1.0f/v.x, 1.0f/v.y, 1.0f/v.z };

    return result;
}

// Clamp the components of the vector between
// min and max values specified by the given vectors
MAPI Vector3 Vector3Clamp(Vector3 v, Vector3 min, Vector3 max)
{
    Vector3 result zeroinit;

    result.x = fminf(max.x, fmaxf(min.x, v.x));
    result.y = fminf(max.y, fmaxf(min.y, v.y));
    result.z = fminf(max.z, fmaxf(min.z, v.z));

    return result;
}

// Clamp the magnitude of the vector between two values
MAPI Vector3 Vector3ClampValue(Vector3 v, float min, float max)
{
    Vector3 result = v;

    float length = (v.x*v.x) + (v.y*v.y) + (v.z*v.z);
    if (length > 0.0f)
    {
        length = sqrtf(length);

        float scale = 1;    // By default, 1 as the neutral element.
        if (length < min)
        {
            scale = min/length;
        }
        else if (length > max)
        {
            scale = max/length;
        }

        result.x = v.x*scale;
        result.y = v.y*scale;
        result.z = v.z*scale;
    }

    return result;
}

// Check whether two given vectors are almost equal
MAPI int Vector3Equals(Vector3 p, Vector3 q)
{
#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    int result = ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                 ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
                 ((fabsf(p.z - q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z)))));

    return result;
}

// Compute the direction of a refracted ray
// v: normalized direction of the incoming ray
// n: normalized normal vector of the interface of two optical media
// r: ratio of the refractive index of the medium from where the ray comes
//    to the refractive index of the medium on the other side of the surface
MAPI Vector3 Vector3Refract(Vector3 v, Vector3 n, float r)
{
    Vector3 result zeroinit;

    float dot = v.x*n.x + v.y*n.y + v.z*n.z;
    float d = 1.0f - r*r*(1.0f - dot*dot);

    if (d >= 0.0f)
    {
        d = sqrtf(d);
        v.x = r*v.x - (r*dot + d)*n.x;
        v.y = r*v.y - (r*dot + d)*n.y;
        v.z = r*v.z - (r*dot + d)*n.z;

        result = v;
    }

    return result;
}

MAPI Quaternion QuaternionAdd(Quaternion q1, Quaternion q2)
{
    Quaternion result = {q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w};

    return result;
}

// Add quaternion and float value
MAPI Quaternion QuaternionAddValue(Quaternion q, float add)
{
    Quaternion result = {q.x + add, q.y + add, q.z + add, q.w + add};

    return result;
}

// Subtract two quaternions
MAPI Quaternion QuaternionSubtract(Quaternion q1, Quaternion q2)
{
    Quaternion result = {q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w};

    return result;
}

// Subtract quaternion and float value
MAPI Quaternion QuaternionSubtractValue(Quaternion q, float sub)
{
    Quaternion result = {q.x - sub, q.y - sub, q.z - sub, q.w - sub};

    return result;
}

// Get identity quaternion
MAPI Quaternion QuaternionIdentity(void)
{
    Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Computes the length of a quaternion
MAPI float QuaternionLength(Quaternion q)
{
    float result = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);

    return result;
}

// Normalize provided quaternion
MAPI Quaternion QuaternionNormalize(Quaternion q)
{
    Quaternion result zeroinit;

    float length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (length == 0.0f) length = 1.0f;
    float ilength = 1.0f/length;

    result.x = q.x*ilength;
    result.y = q.y*ilength;
    result.z = q.z*ilength;
    result.w = q.w*ilength;

    return result;
}

// Invert provided quaternion
MAPI Quaternion QuaternionInvert(Quaternion q)
{
    Quaternion result = q;

    float lengthSq = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;

    if (lengthSq != 0.0f)
    {
        float invLength = 1.0f/lengthSq;

        result.x *= -invLength;
        result.y *= -invLength;
        result.z *= -invLength;
        result.w *= invLength;
    }

    return result;
}

// Calculate two quaternion multiplication
MAPI Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2)
{
    Quaternion result zeroinit;

    float qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
    float qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;

    result.x = qax*qbw + qaw*qbx + qay*qbz - qaz*qby;
    result.y = qay*qbw + qaw*qby + qaz*qbx - qax*qbz;
    result.z = qaz*qbw + qaw*qbz + qax*qby - qay*qbx;
    result.w = qaw*qbw - qax*qbx - qay*qby - qaz*qbz;

    return result;
}

// Scale quaternion by float value
MAPI Quaternion QuaternionScale(Quaternion q, float mul)
{
    Quaternion result zeroinit;

    result.x = q.x*mul;
    result.y = q.y*mul;
    result.z = q.z*mul;
    result.w = q.w*mul;

    return result;
}

// Divide two quaternions
MAPI Quaternion QuaternionDivide(Quaternion q1, Quaternion q2)
{
    Quaternion result = { q1.x/q2.x, q1.y/q2.y, q1.z/q2.z, q1.w/q2.w };

    return result;
}

// Calculate linear interpolation between two quaternions
MAPI Quaternion QuaternionLerp(Quaternion q1, Quaternion q2, float amount)
{
    Quaternion result zeroinit;

    result.x = q1.x + amount*(q2.x - q1.x);
    result.y = q1.y + amount*(q2.y - q1.y);
    result.z = q1.z + amount*(q2.z - q1.z);
    result.w = q1.w + amount*(q2.w - q1.w);

    return result;
}

// Calculate slerp-optimized interpolation between two quaternions
MAPI Quaternion QuaternionNlerp(Quaternion q1, Quaternion q2, float amount)
{
    Quaternion result zeroinit;

    // QuaternionLerp(q1, q2, amount)
    result.x = q1.x + amount*(q2.x - q1.x);
    result.y = q1.y + amount*(q2.y - q1.y);
    result.z = q1.z + amount*(q2.z - q1.z);
    result.w = q1.w + amount*(q2.w - q1.w);

    // QuaternionNormalize(q);
    Quaternion q = result;
    float length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (length == 0.0f) length = 1.0f;
    float ilength = 1.0f/length;

    result.x = q.x*ilength;
    result.y = q.y*ilength;
    result.z = q.z*ilength;
    result.w = q.w*ilength;

    return result;
}

// Calculates spherical linear interpolation between two quaternions
MAPI Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float amount)
{
    Quaternion result zeroinit;

#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    float cosHalfTheta = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    if (cosHalfTheta < 0)
    {
        q2.x = -q2.x; q2.y = -q2.y; q2.z = -q2.z; q2.w = -q2.w;
        cosHalfTheta = -cosHalfTheta;
    }

    if (fabsf(cosHalfTheta) >= 1.0f) result = q1;
    else if (cosHalfTheta > 0.95f) result = QuaternionNlerp(q1, q2, amount);
    else
    {
        float halfTheta = acosf(cosHalfTheta);
        float sinHalfTheta = sqrtf(1.0f - cosHalfTheta*cosHalfTheta);

        if (fabsf(sinHalfTheta) < EPSILON)
        {
            result.x = (q1.x*0.5f + q2.x*0.5f);
            result.y = (q1.y*0.5f + q2.y*0.5f);
            result.z = (q1.z*0.5f + q2.z*0.5f);
            result.w = (q1.w*0.5f + q2.w*0.5f);
        }
        else
        {
            float ratioA = sinf((1 - amount)*halfTheta)/sinHalfTheta;
            float ratioB = sinf(amount*halfTheta)/sinHalfTheta;

            result.x = (q1.x*ratioA + q2.x*ratioB);
            result.y = (q1.y*ratioA + q2.y*ratioB);
            result.z = (q1.z*ratioA + q2.z*ratioB);
            result.w = (q1.w*ratioA + q2.w*ratioB);
        }
    }

    return result;
}
MAPI void MatrixDecompose(Matrix mat, Vector3 *translation, Quaternion *rotation, Vector3 *scale){
    // Extract translation.
    translation->x = mat.data[12];
    translation->y = mat.data[13];
    translation->z = mat.data[14];

    // Extract upper-left for determinant computation
    const float a = mat.data[0];
    const float b = mat.data[4];
    const float c = mat.data[8];
    const float d = mat.data[1];
    const float e = mat.data[5];
    const float f = mat.data[9];
    const float g = mat.data[2];
    const float h = mat.data[6];
    const float i = mat.data[10];
    const float A = e*i - f*h;
    const float B = f*g - d*i;
    const float C = d*h - e*g;

    // Extract scale
    const float det = a*A + b*B + c*C;
    Vector3 abc = { a, b, c };
    Vector3 def = { d, e, f };
    Vector3 ghi = { g, h, i };

    float scalex = sqrtf(abc.x*abc.x + abc.y * abc.y + abc.z * abc.z);
    float scaley = sqrtf(def.x*def.x + def.y * def.y + def.z * def.z);
    float scalez = sqrtf(ghi.x*ghi.x + ghi.y * ghi.y + ghi.z * ghi.z);
    Vector3 s = { scalex, scaley, scalez };

    if (det < 0) s = Vector3Negate(s);

    *scale = s;

    // Remove scale from the matrix if it is not close to zero
    Matrix clone = mat;
    if (!FloatEquals(det, 0))
    {
        clone.data[0 ] /= s.x;
        clone.data[4 ] /= s.x;
        clone.data[8 ] /= s.x;
        clone.data[1 ] /= s.y;
        clone.data[5 ] /= s.y;
        clone.data[9 ] /= s.y;
        clone.data[2 ] /= s.z;
        clone.data[6 ] /= s.z;
        clone.data[10] /= s.z;

        // Extract rotation
        *rotation = QuaternionFromMatrix(clone);
    }
    else
    {
        // Set to identity if close to zero
        *rotation = CLITERAL(Quaternion){0.0f, 0.0f, 0.0f, 1.0f };
    }
}
MAPI Quaternion QuaternionCubicHermiteSpline(Quaternion q1, Quaternion outTangent1, Quaternion q2, Quaternion inTangent2, float t){
    float t2 = t*t;
    float t3 = t2*t;
    float h00 = 2*t3 - 3*t2 + 1;
    float h10 = t3 - 2*t2 + t;
    float h01 = -2*t3 + 3*t2;
    float h11 = t3 - t2;

    Quaternion p0 = QuaternionScale(q1, h00);
    Quaternion m0 = QuaternionScale(outTangent1, h10);
    Quaternion p1 = QuaternionScale(q2, h01);
    Quaternion m1 = QuaternionScale(inTangent2, h11);

    Quaternion result zeroinit;

    result = QuaternionAdd(p0, m0);
    result = QuaternionAdd(result, p1);
    result = QuaternionAdd(result, m1);
    result = QuaternionNormalize(result);

    return result;
}
MAPI Matrix MatrixInvert(Matrix mat){
    Matrix result zeroinit;

    float a00 = mat.data[0 ], a01 = mat.data[1 ], a02 = mat.data[2 ], a03 = mat.data[3 ];
    float a10 = mat.data[4 ], a11 = mat.data[5 ], a12 = mat.data[6 ], a13 = mat.data[7 ];
    float a20 = mat.data[8 ], a21 = mat.data[9 ], a22 = mat.data[10], a23 = mat.data[11];
    float a30 = mat.data[12], a31 = mat.data[13], a32 = mat.data[14], a33 = mat.data[15];

    float b00 = a00*a11 - a01*a10;
    float b01 = a00*a12 - a02*a10;
    float b02 = a00*a13 - a03*a10;
    float b03 = a01*a12 - a02*a11;
    float b04 = a01*a13 - a03*a11;
    float b05 = a02*a13 - a03*a12;
    float b06 = a20*a31 - a21*a30;
    float b07 = a20*a32 - a22*a30;
    float b08 = a20*a33 - a23*a30;
    float b09 = a21*a32 - a22*a31;
    float b10 = a21*a33 - a23*a31;
    float b11 = a22*a33 - a23*a32;

    float invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    result.data[0 ] = (a11*b11 - a12*b10 + a13*b09)*invDet;
    result.data[1 ] = (-a01*b11 + a02*b10 - a03*b09)*invDet;
    result.data[2 ] = (a31*b05 - a32*b04 + a33*b03)*invDet;
    result.data[3 ] = (-a21*b05 + a22*b04 - a23*b03)*invDet;
    result.data[4 ] = (-a10*b11 + a12*b08 - a13*b07)*invDet;
    result.data[5 ] = (a00*b11 - a02*b08 + a03*b07)*invDet;
    result.data[6 ] = (-a30*b05 + a32*b02 - a33*b01)*invDet;
    result.data[7 ] = (a20*b05 - a22*b02 + a23*b01)*invDet;
    result.data[8 ] = (a10*b10 - a11*b08 + a13*b06)*invDet;
    result.data[9 ] = (-a00*b10 + a01*b08 - a03*b06)*invDet;
    result.data[10] = (a30*b04 - a31*b02 + a33*b00)*invDet;
    result.data[11] = (-a20*b04 + a21*b02 - a23*b00)*invDet;
    result.data[12] = (-a10*b09 + a11*b07 - a12*b06)*invDet;
    result.data[13] = (a00*b09 - a01*b07 + a02*b06)*invDet;
    result.data[14] = (-a30*b03 + a31*b01 - a32*b00)*invDet;
    result.data[15] = (a20*b03 - a21*b01 + a22*b00)*invDet;

    return result;
}
MAPI Matrix MatrixTranslate(float x, float y, float z){    
    return CLITERAL(Matrix){
        1,0,0,0,// First !! column !!
        0,1,0,0,
        0,0,1,0,
        x,y,z,1,
    };
}
MAPI Matrix MatrixScale(float x, float y, float z){
    return CLITERAL(Matrix){
        x,0,0,0,// First !! column !!
        0,y,0,0,
        0,0,z,0,
        0,0,0,1,
    };
}
MAPI Matrix MatrixRotate(Vector3 axis, float angle){
    Matrix result zeroinit;

    float x = axis.x, y = axis.y, z = axis.z;

    float lengthSquared = x * x + y * y + z * z;

    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f))
    {
        float ilength = 1.0f / sqrtf(lengthSquared);
        x *= ilength;
        y *= ilength;
        z *= ilength;
    }

    float sinres = sinf(angle * DEG2RAD);
    float cosres = cosf(angle * DEG2RAD);
    float t = 1.0f - cosres;

    result.data[0 ] = x * x * t + cosres;
    result.data[1 ] = y * x * t + z * sinres;
    result.data[2 ] = z * x * t - y * sinres;
    result.data[3 ] = 0.0f;
    result.data[4 ] = x * y * t - z * sinres;
    result.data[5 ] = y * y * t + cosres;
    result.data[6 ] = z * y * t + x * sinres;
    result.data[7 ] = 0.0f;
    result.data[8 ] = x * z * t + y * sinres;
    result.data[9 ] = y * z * t - x * sinres;
    result.data[10] = z * z * t + cosres;
    result.data[11] = 0.0f;
    result.data[12] = 0.0f;
    result.data[13] = 0.0f;
    result.data[14] = 0.0f;
    result.data[15] = 1.0f;

    return result;
}

MAPI Matrix GetCameraMatrix3D(Camera3D camera, float aspect){
    return MatrixMultiply(MatrixPerspective(camera.fovy, aspect , 0.1f, 100000), MatrixLookAt(camera.position, camera.target, camera.up));
}

MAPI Matrix GetCameraMatrix2D(Camera2D camera){
    Matrix matTransform zeroinit;
    // The camera in world-space is set by
    //   1. Move it to target
    //   2. Rotate by -rotation and scale by (1/zoom)
    //      When setting higher scale, it's more intuitive for the world to become bigger (= camera become smaller),
    //      not for the camera getting bigger, hence the invert. Same deal with rotation
    //   3. Move it by (-offset);
    //      Offset defines target transform relative to screen, but since we're effectively "moving" screen (camera)
    //      we need to do it into opposite direction (inverse transform)

    // Having camera transform in world-space, inverse of it gives the modelview transform
    // Since (A*B*C)' = C'*B'*A', the modelview is
    //   1. Move to offset
    //   2. Rotate and Scale
    //   3. Move by -target
    Matrix matOrigin = MatrixTranslate(-camera.target.x, -camera.target.y, 0.0f);
    Matrix matRotation = MatrixRotate(CLITERAL(Vector3){ 0.0f, 0.0f, 1.0f}, camera.rotation);
    Matrix matScale = MatrixScale(camera.zoom, camera.zoom, 1.0f);
    Matrix matTranslation = MatrixTranslate(camera.offset.x, camera.offset.y, 0.0f);

    matTransform = MatrixMultiplySwap(MatrixMultiplySwap(matOrigin, MatrixMultiplySwap(matScale, matRotation)), matTranslation);

    return matTransform;
}

#ifdef __cplusplus
MAPI Vector3 operator*(const Matrix& m, const Vector3 v){
    Vector3 ret zeroinit;
    ret.x += v.x * m.data[0]; //First column 
    ret.y += v.x * m.data[1]; //First column 
    ret.z += v.x * m.data[2]; //First column

    ret.x += v.y * m.data[4];
    ret.y += v.y * m.data[5];
    ret.z += v.y * m.data[6];

    ret.x += v.z * m.data[8];
    ret.y += v.z * m.data[9];
    ret.z += v.z * m.data[10];

    ret.x += m.data[12];
    ret.y += m.data[13];
    ret.z += m.data[14];
    return ret;
}
MAPI std::ostream& operator<<(std::ostream& str, const Vector2& r){
    str << "["; 
    str << r.x << ", ";
    str << r.y << ", ";
    return str << "]"; 
}
MAPI std::ostream& operator<<(std::ostream& str, const Vector3& r){
    str << "["; 
    str << r.x << ", ";
    str << r.y << ", ";
    str << r.z << ", ";
    return str << "]"; 
}
MAPI std::ostream& operator<<(std::ostream& str, const Vector4& r){
    str << "["; 
    str << r.x << ", ";
    str << r.y << ", ";
    str << r.z << ", ";
    str << r.w << ", ";
    return str << "]"; 
}
MAPI std::ostream& operator<<(std::ostream& str, const Matrix& m){
    str << m.data[0 ] << ", ";
    str << m.data[4 ] << ", ";
    str << m.data[8 ] << ", ";
    str << m.data[12] << ", ";
    str << '\n';
    str << m.data[1 + 0 ] << ", ";
    str << m.data[1 + 4 ] << ", ";
    str << m.data[1 + 8 ] << ", ";
    str << m.data[1 + 12] << ", ";
    str << '\n';
    str << m.data[2 + 0 ] << ", ";
    str << m.data[2 + 4 ] << ", ";
    str << m.data[2 + 8 ] << ", ";
    str << m.data[2 + 12] << ", ";
    str << '\n';
    str << m.data[3 + 0 ] << ", ";
    str << m.data[3 + 4 ] << ", ";
    str << m.data[3 + 8 ] << ", ";
    str << m.data[3 + 12] << ", ";
    str << '\n';
    return str; 
}
#endif //__cplusplus

#endif // Include guard
