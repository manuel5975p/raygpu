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
        Matrix ret = {0};
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

typedef struct Quaternion{
    float x,y,z,w;
}Quaternion;

typedef struct Transform {
    Vector3 translation;    // Translation
    Quaternion rotation;    // Rotation
    Vector3 scale;          // Scale
} Transform;

MAPI Matrix MatrixIdentity(void){
    Matrix ret = {0};
    ret.data[0] = 1;
    ret.data[5] = 1;
    ret.data[10] = 1;
    ret.data[15] = 1;
    return ret;
}


MAPI Matrix MatrixMultiply(Matrix A, Matrix B){
    Matrix ret = {0};
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
    Matrix result = {0};

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
    Matrix ret = {0};
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            ret.data[j * 4 + i] = mat.data[i * 4 + j];
        }
    }
    return ret;
}
MAPI Vector4 Vector4Negate(Vector4 x){
    return CLITERAL(Vector4){-x.x, -x.y, -x.z, x.w};
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
}// Get a quaternion for a given rotation matrix
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
    return MatrixMultiply(MatrixPerspective(camera.fovy, aspect , 0.01, 1000), MatrixLookAt(camera.position, camera.target, camera.up));
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
MAPI std::ostream& operator<<(std::ostream& str, const Vector3& r){
    str << "["; 
    str << r.x << ", ";
    str << r.y << ", ";
    str << r.z << ", ";
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
