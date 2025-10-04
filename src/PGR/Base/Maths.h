#pragma once

#include "PGR/Base/Base.h"

#include <cmath>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>

namespace PGR {

    constexpr float PI = 3.14159265359f;
    constexpr float EPSILON = 1e-5f;

    struct Vec2 {
        float X, Y;

        constexpr Vec2()
            : X(0.0f), Y(0.0f) {}
        constexpr Vec2(float x, float y)
            : X(x), Y(y) {}
    };

    struct Vec3 {
        float X, Y, Z;

        constexpr Vec3()
            : X(0.0f), Y(0.0f), Z(0.0f) {}
        constexpr Vec3(float val)
            : X(val), Y(val), Z(val) {}
        constexpr Vec3(Vec2 v, float z)
            : X(v.X), Y(v.Y), Z(z) {}
        constexpr Vec3(float x, float y, float z)
            : X(x), Y(y), Z(z) {}

        operator Vec2() const { return { X, Y }; }

    };

    struct Vec4 {
        float X, Y, Z, W;

        constexpr Vec4()
            : X(0.0f), Y(0.0f), Z(0.0f), W(0.0f) {}
        constexpr Vec4(float val)
            : X(val), Y(val), Z(val), W(val) {}
        constexpr Vec4(float x, float y, float z, float w)
            : X(x), Y(y), Z(z), W(w) {}
        constexpr Vec4(const Vec2& vec2, float z, float w)
            : X(vec2.X), Y(vec2.Y), Z(z), W(w) {}
        constexpr Vec4(const Vec3& vec3, float w)
            : X(vec3.X), Y(vec3.Y), Z(vec3.Z), W(w) {}

        operator Vec2() const { return { X, Y }; }
        operator Vec3() const { return { X, Y, Z }; }
    };

    Vec2 operator+ (const Vec2& left, const Vec2& right);
    Vec2 operator- (const Vec2& left, const Vec2& right);

    Vec3 operator+ (const Vec3& left, const Vec3& right);
    Vec3 operator- (const Vec3& left, const Vec3& right);
    Vec3 operator* (const float left, const Vec3& right);
    Vec3 operator* (const Vec3& left, const float right);
    Vec3 operator* (const Vec3& left, const Vec3& right);
    Vec3 operator/ (const Vec3& left, const float right);
    Vec3 operator/ (const Vec3& left, const Vec3& right);
    Vec3& operator*= (Vec3& left, const float right);
    Vec3& operator/= (Vec3& left, const float right);
    Vec3& operator+= (Vec3& left, const Vec3& right);

    float Dot(const Vec3& left, const Vec3& right);
    Vec3 Cross(const Vec3& left, const Vec3& right);
    Vec3 Normalize(const Vec3&);
    Vec3 Reflect(const Vec3& in, const Vec3& normal);

    Vec4 operator+ (const Vec4& left, const Vec4& right);
    Vec4 operator- (const Vec4& left, const Vec4& right);
    Vec4 operator* (const float left, const Vec4& right);
    Vec4 operator* (const Vec4& left, const float right);
    Vec4 operator* (const Vec4& left, const Vec4& right);
    Vec4 operator/ (const Vec4& left, const float right);

    Vec4& operator+= (Vec4& left, const Vec4& right);
    Vec4& operator-= (Vec4& left, const Vec4& right);

    float Lerp(const float start, const float end, const float t);
    Vec3 Lerp(const Vec3& start, const Vec3& end, const float t);
    Vec4 Lerp(const Vec4& start, const Vec4& end, const float t);

    float Clamp(const float in, const float min, const float max);
    Vec3 Clamp(const Vec3& vec, const float min, const float max);
    Vec4 Clamp(const Vec4& vec, const float min, const float max);

    Vec3 Pow(const Vec3& vec, const float exponent);

    float Length(const Vec3& v);

    unsigned char Float2UChar(const float f);
    float UChar2Float(const unsigned char c);

    float Max(const float right, const float left);
    float Min(const float right, const float left);
}