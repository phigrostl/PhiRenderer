#include "Maths.h"

namespace PGR {

    Vec2 operator+ (const Vec2& left, const Vec2& right) {
        return Vec2{ left.X + right.X, left.Y + right.Y };
    }
    Vec2 operator- (const Vec2& left, const Vec2& right) {
        return Vec2{ left.X - right.X, left.Y - right.Y };
    }

    Vec3 operator+ (const Vec3& left, const Vec3& right) {
        return Vec3{ left.X + right.X, left.Y + right.Y, left.Z + right.Z };
    }
    Vec3 operator- (const Vec3& left, const Vec3& right) {
        return left + (-1.0f * right);
    }
    Vec3 operator* (const float left, const Vec3& right) {
        return Vec3{ left * right.X, left * right.Y, left * right.Z };
    }
    Vec3 operator* (const Vec3& left, const float right) {
        return right * left;
    }
    Vec3 operator* (const Vec3& left, const Vec3& right) {
        return { left.X * right.X, left.Y * right.Y, left.Z * right.Z };
    }
    Vec3 operator/ (const Vec3& left, const float right) {
        ASSERT((right != 0));
        return left * (1.0f / right);
    }
    Vec3 operator/ (const Vec3& left, const Vec3& right) {
        ASSERT((right.X != 0) && (right.Y != 0) && (right.Z != 0));
        return left * Vec3{ 1.0f / right.X, 1.0f / right.Y, 1.0f / right.Z };
    }
    Vec3& operator*= (Vec3& left, const float right) {
        left = left * right;
        return left;
    }
    Vec3& operator/= (Vec3& left, const float right) {
        ASSERT((right != 0));
        left = left / right;
        return left;
    }
    Vec3& operator+= (Vec3& left, const Vec3& right) {
        left = left + right;
        return left;
    }
    Vec3 Cross(const Vec3& left, const Vec3& right) {
        float x = left.Y * right.Z - left.Z * right.Y;
        float y = left.Z * right.X - left.X * right.Z;
        float z = left.X * right.Y - left.Y * right.X;
        return { x, y, z };
    }
    float Dot(const Vec3& left, const Vec3& right) {
        return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
    }

    Vec4 operator+ (const Vec4& left, const Vec4& right) {
        return Vec4{ left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W };
    }
    Vec4 operator- (const Vec4& left, const Vec4& right) {
        return Vec4{ left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W };
    }
    Vec4 operator* (const float left, const Vec4& right) {
        return Vec4{ left * right.X, left * right.Y, left * right.Z, left * right.W };
    }
    Vec4 operator* (const Vec4& left, const float right) {
        return right * left;
    }
    Vec4 operator/ (const Vec4& left, const float right) {
        ASSERT(right != 0);
        return left * (1.0f / right);
    }

    Vec4& operator+= (Vec4& left, const Vec4& right) {
        left.X += right.X;
        left.Y += right.Y;
        left.Z += right.Z;
        left.W += right.W;
        return left;
    }

    Vec4& operator-= (Vec4& left, const Vec4& right) {
        left.X -= right.X;
        left.Y -= right.Y;
        left.Z -= right.Z;
        left.W -= right.W;
        return left;
    }

    Vec3 Normalize(const Vec3& v) {
        float len = (float)std::sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
        ASSERT((len != 0));
        return v / len;
    }

    Vec3 Reflect(const Vec3& in, const Vec3& normal) {
        return -2 * Dot(in, normal) * normal + in;
    }

    float Clamp(const float val, const float min, const float max) {
        return std::max(min, std::min(val, max));
    }

    Vec3 Clamp(const Vec3& vec, const float min, const float max) {
        return Vec3{
            std::max(min, std::min(vec.X, max)),
            std::max(min, std::min(vec.Y, max)),
            std::max(min, std::min(vec.Z, max))
        };
    }

    Vec4 Clamp(const Vec4& vec, const float min, const float max) {
        return Vec4{
            std::max(min, std::min(vec.X, max)),
            std::max(min, std::min(vec.Y, max)),
            std::max(min, std::min(vec.Z, max)),
            std::max(min, std::min(vec.W, max))
        };
    }

    Vec3 Pow(const Vec3& vec, const float exponent) {
        return Vec3{
            std::pow(vec.X, exponent),
            std::pow(vec.Y, exponent),
            std::pow(vec.Z, exponent)
        };
    }

    float Length(const Vec3& v) {
        return (float)std::sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
    }

    float Lerp(const float start, const float end, const float t) {
        return end * t + start * (1.0f - t);
    }

    Vec3 Lerp(const Vec3& start, const Vec3& end, const float t) {
        return end * t + start * (1.0f - t);
    }

    Vec4 Lerp(const Vec4& start, const Vec4& end, const float t)
    {
        return end * t + start * (1.0f - t);
    }

    unsigned char Float2UChar(const float f) {
        return static_cast<unsigned char>(f * 255.0f + 0.5f);
    }

    float UChar2Float(const unsigned char c) {
        return (float)c / 255.0f;
    }

    float Max(const float right, const float left) {
        return std::max<float>(right, left);
    }

    float Min(const float right, const float left) {
        return std::min<float>(right, left);
    }

}
