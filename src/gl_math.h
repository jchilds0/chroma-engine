/*
 *
 */

#include <math.h>

#define DEG_TO_RAD(theta)     (theta * M_PI / 180)

#define GL_MATH_ROTATE_X(theta) {\
    1, 0,           0,            0, \
    0, cosf(theta), -sinf(theta), 0, \
    0, sinf(theta), cosf(theta),  0, \
    0, 0,           0,            1}

#define GL_MATH_ROTATE_Y(theta) {\
    cosf(theta), 0, -sinf(theta), 0, \
    0,           1, 0,            0, \
    sinf(theta), 0, cosf(theta),  0, \
    0,           0,           0,  1}

#define GL_MATH_ROTATE_Z(theta)         {\
    cosf(theta),-sinf(theta), 0, 0, \
    sinf(theta), cosf(theta), 0, 0, \
    0          , 0          , 1, 0, \
    0          , 0          , 0, 1}

#define GL_MATH_ORTHO(left, right, bottom, top, zNear, zFar)    {\
    2 / (right - left), 0,                  0,                  (right + left) / (left - right), \
    0,                  2 / (top - bottom), 0,                  (top + bottom) / (bottom - top), \
    0,                  0,                  2 / (zNear - zFar), (zFar + zNear) / (zNear - zFar), \
    0,                  0,                  0,                  1}

#define FOV_TO_F(fov)      (1.0 / tanf(fov / 2.0))

#define GL_MATH_PERSPECTIVE(fov, aspect, zNear, zFar)    {\
    FOV_TO_F(fov) / (aspect), 0.0,             0.0,                             0.0, \
    0.0,                      FOV_TO_F(fov),   0.0,                             0.0, \
    0.0,                      0.0,             (zFar + zNear) / (zNear - zFar), (2 * zFar * zNear) / (zNear - zFar), \
    0.0,                      0.0,             -1.0,                            0.0}

#define GL_MATH_TRANSLATE(x, y, z) {\
    1.0, 0.0, 0.0, x, \
    0.0, 1.0, 0.0, y, \
    0.0, 0.0, 1.0, z, \
    0.0, 0.0, 0.0, 1.0}

#define GL_MATH_ID        { 1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1 }

#define GL_MATH_MULT(A, B) {\
    A[0] * B[0] + A[1] * B[4] + A[2] * B[8] + A[3] * B[12], \
    A[0] * B[1] + A[1] * B[5] + A[2] * B[9] + A[3] * B[13], \
    A[0] * B[2] + A[1] * B[6] + A[2] * B[10] + A[3] * B[14], \
    A[0] * B[3] + A[1] * B[7] + A[2] * B[11] + A[3] * B[15], \
    \
    A[4] * B[0] + A[5] * B[4] + A[6] * B[8] + A[7] * B[12], \
    A[4] * B[1] + A[5] * B[5] + A[6] * B[9] + A[7] * B[13], \
    A[4] * B[2] + A[5] * B[6] + A[6] * B[10] + A[7] * B[14], \
    A[4] * B[3] + A[5] * B[7] + A[6] * B[11] + A[7] * B[15], \
    \
    A[8] * B[0] + A[9] * B[4] + A[10] * B[8] + A[11] * B[12], \
    A[8] * B[1] + A[9] * B[5] + A[10] * B[9] + A[11] * B[13], \
    A[8] * B[2] + A[9] * B[6] + A[10] * B[10] + A[11] * B[14], \
    A[8] * B[3] + A[9] * B[7] + A[10] * B[11] + A[11] * B[15], \
    \
    A[12] * B[0] + A[13] * B[4] + A[14] * B[8] + A[15] * B[12], \
    A[12] * B[1] + A[13] * B[5] + A[14] * B[9] + A[15] * B[13], \
    A[12] * B[2] + A[13] * B[6] + A[14] * B[10] + A[15] * B[14], \
    A[12] * B[3] + A[13] * B[7] + A[14] * B[11] + A[15] * B[15]} 


