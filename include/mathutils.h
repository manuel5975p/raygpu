#ifndef CAMERA_H
#define CAMERA_H
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
 **********************************************************************************************/



#ifdef __cplusplus
#include <cmath>
using std::sqrtf;
using std::tan;
using std::tanf;
#else
#include <math.h>
#endif

typedef struct Vector4{
    float x,y,z,w;
}Vector4;

typedef struct Vector3{
    float x,y,z;
}Vector3;

typedef struct Vector2{
    float x,y;
}Vector2;

inline Vector2 Vector2Normalize(Vector2 vec){
    float one_by_nrm = 1.0f / sqrt(vec.x * vec.x + vec.y * vec.y);
    vec.x *= one_by_nrm;
    vec.y *= one_by_nrm;
    return vec;
}
inline Vector3 Vector3Normalize(Vector3 vec){
    float one_by_nrm = 1.0f / sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    vec.x *= one_by_nrm;
    vec.y *= one_by_nrm;
    vec.z *= one_by_nrm;
    return vec;
}
inline Vector4 Vector4Normalize(Vector4 vec){
    float one_by_nrm = 1.0f / sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
    vec.x *= one_by_nrm;
    vec.y *= one_by_nrm;
    vec.z *= one_by_nrm;
    vec.w *= one_by_nrm;
    return vec;
}

typedef struct Matrix{
    float data[16];
} Matrix;
inline Matrix MatrixIdentity(void){
    Matrix ret = {0};
    ret.data[0] = 1;
    ret.data[5] = 1;
    ret.data[10] = 1;
    ret.data[15] = 1;
    return ret;
}
inline Matrix MatrixMultiply(Matrix A, Matrix B){
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

inline Vector3 Vector3Subtract(Vector3 v1, Vector3 v2){
    Vector3 result = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return result;
}

// https://github.com/raysan5/raylib/blob/master/src/raymath.h
inline Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up){
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
inline Matrix MatrixPerspective(double fovY, double aspect, double nearPlane, double farPlane){
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
inline Matrix ScreenMatrix(int width, int height) {
    Matrix ret = MatrixIdentity();
    ret.data[idx_map(0, 3)] = -1.0f;
    ret.data[idx_map(1, 3)] = -1.0f;
    ret.data[0] = 2.0f / ((float)width);
    ret.data[5] = 2.0f / ((float)height);
    ret.data[10] = 1;
    ret.data[15] = 1;
    return ret;
}

inline Matrix MatrixTranspose(Matrix mat){
    Matrix ret = {0};
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            ret.data[j * 4 + i] = mat.data[i * 4 + j];
        }
    }
    return ret;
}
#endif
