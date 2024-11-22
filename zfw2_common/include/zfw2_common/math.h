#pragma once

namespace zfw2_common
{

struct Vec2D
{
    float x;
    float y;
};

struct Vec2DInt
{
    int x;
    int y;
};

struct Matrix4x4
{
    float elems[4][4];

    static inline Matrix4x4 create_identity()
    {
        Matrix4x4 mat = {};
        mat[0][0] = 1.0f;
        mat[1][1] = 1.0f;
        mat[2][2] = 1.0f;
        mat[3][3] = 1.0f;
        return mat;
    }

    static inline Matrix4x4 create_ortho(const float left, const float right, const float bottom, const float top, const float near, const float far)
    {
        Matrix4x4 mat = {};
        mat[0][0] = 2.0f / (right - left);
        mat[1][1] = 2.0f / (top - bottom);
        mat[2][2] = -2.0f / (far - near);
        mat[3][0] = -(right + left) / (right - left);
        mat[3][1] = -(top + bottom) / (top - bottom);
        mat[3][2] = -(far + near) / (far - near);
        mat[3][3] = 1.0f;
        return mat;
    }

    float *operator[](const int index)
    {
        return elems[index];
    }
};

struct Rect
{
    int x;
    int y;
    int width;
    int height;
};

}
