#pragma once

namespace zfw2_common
{

struct Vec2D
{
    float x;
    float y;

    Vec2D operator+(const Vec2D& other) const
    {
        return {x + other.x, y + other.y};
    }

    Vec2D operator-(const Vec2D& other) const
    {
        return {x - other.x, y - other.y};
    }

    Vec2D operator*(const float scalar) const
    {
        return {x * scalar, y * scalar};
    }

    Vec2D operator/(const float scalar) const
    {
        return {x / scalar, y / scalar};
    }

    bool operator==(const Vec2D& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vec2D& other) const
    {
        return !(*this == other);
    }

    Vec2D& operator+=(const Vec2D& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2D& operator-=(const Vec2D& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2D& operator*=(const float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2D& operator/=(const float scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }
};

struct Vec2DInt
{
    int x;
    int y;

    Vec2DInt operator+(const Vec2DInt& other) const
    {
        return {x + other.x, y + other.y};
    }

    Vec2DInt operator-(const Vec2DInt& other) const
    {
        return {x - other.x, y - other.y};
    }

    Vec2DInt operator*(const int scalar) const
    {
        return {x * scalar, y * scalar};
    }

    Vec2DInt operator/(const int scalar) const
    {
        return {x / scalar, y / scalar};
    }

    bool operator==(const Vec2DInt& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vec2DInt& other) const
    {
        return !(*this == other);
    }

    Vec2DInt& operator+=(const Vec2DInt& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2DInt& operator-=(const Vec2DInt& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2DInt& operator*=(const int scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2DInt& operator/=(const int scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }
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

struct RectFloat
{
    float x;
    float y;
    float width;
    float height;
};

constexpr float gk_pi = 3.14159265358979323846f;

}
