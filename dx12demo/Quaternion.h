#pragma once

#include "SimpleMath.h"
#include <iostream>

namespace Framework
{
    class Quaternion
    {
    public:
        Quaternion(void);
        Quaternion(float i, float j, float k, float s);
        Quaternion(DirectX::SimpleMath::Vector3 v, float s);
        ~Quaternion(void);

        DirectX::SimpleMath::Vector3 v;
        float s = 0.0f;

        // dot product
        const float Dot(const Quaternion& rhs) const;

        // length squared
        const float LengthSquared(void) const;

        // length
        const float Length(void) const;

        // apply a x,y,z,w component-wise division by length
        void Normalize(void);

        // returns a copy of the normalised quaternion
        Quaternion GetNormalizedCopy(void) const;

        // apply a component-wise multiplication of -1 to x, y, z
        void Conjugate(void);

        // returns a copy of the copied quaternion
        Quaternion GetConjugateCopy(void) const;

        // normalise the conjugated quaternion. Inverses the rotation.
        Quaternion Inverse(void) const;

        // 0, 0, 0, 1
        static const Quaternion Identity;

        // x,y,z = axis.(x,y,z) * sin(angle/2)
        // w = cos(angle/2)
        static Quaternion CreateFromAxisAngle(const DirectX::SimpleMath::Vector3& axis, float angle);

        DirectX::SimpleMath::Matrix ToMatrix(void) const;
    };

    inline Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
    {
        Quaternion r = lhs;
        r.v += rhs.v;
        r.s += rhs.s;
        return r;
    }

    inline Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs)
    {
        Quaternion r = lhs;
        r.v -= rhs.v;
        r.s -= rhs.s;
        return r;
    }

    // Combines two rotations. Order is significant.
    inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
    {
        const float x = rhs.s * lhs.v.x + rhs.v.x * lhs.s + rhs.v.y * lhs.v.z - rhs.v.z * lhs.v.y;
        const float y = rhs.s * lhs.v.y - rhs.v.x * lhs.v.z + rhs.v.y * lhs.s + rhs.v.z * lhs.v.x;
        const float z = rhs.s * lhs.v.z + rhs.v.x * lhs.v.y - rhs.v.y * lhs.v.x + rhs.v.z * lhs.s;
        const float s = rhs.s * lhs.s - rhs.v.x * lhs.v.x - rhs.v.y * lhs.v.y - rhs.v.z * lhs.v.z;

        Quaternion r;
        r.v = DirectX::SimpleMath::Vector3(x, y, z);
        r.s = s;
        return r;
    }

    // any scalar multiple of q represents the same rotation
    inline Quaternion operator*(const Quaternion& q, float s)
    {
        Quaternion r = q;
        r.v *= s;
        r.s *= s;
        return r;
    }

    inline Quaternion operator*(float s, const Quaternion& q)
    {
        Quaternion r = q;
        r.v *= s;
        r.s *= s;
        return r;
    }

    // apply the inverse of the rhs quaternion to the lhs quaternion
    inline Quaternion operator/ (const Quaternion& lhs, const Quaternion& rhs)
    {
        const Quaternion r = lhs * rhs.Inverse();
        return r;
    }

    // rotate a vector by a quaternion q r inverse(q)
    inline DirectX::SimpleMath::Vector3 Rotate(const Quaternion& q, const DirectX::SimpleMath::Vector3& v)
    {
        // do hamilton product on vector as a pure quaternion
        const Quaternion r = Quaternion(v, 0.f);
        const Quaternion qr = r * q;
        const Quaternion qrqi = q.Inverse() * qr;
        return qrqi.v;
    }

    // linearly interpolate between two quaternions
    inline Quaternion Lerp(const Quaternion& start, const Quaternion end, float t)
    {
        float multiplier = 1.0f;
        if (start.Dot(end) < 0.0f) multiplier = -1.0f;
        const Quaternion r = (1.0f - t) * start + t * (end * multiplier);
        return r;
    }

    // linearly interpolate between two quaternion, then normalise
    inline Quaternion LerpNormalize(const Quaternion& start, const Quaternion end, float t)
    {
        Quaternion r = Lerp(start, end, t);
        r.Normalize();
        return r;
    }

    // spherical interpolation between two quaternions
    inline Quaternion Slerp(const Quaternion& start, const Quaternion end, float t)
    {
        Quaternion r;
        const float slerp_epsilon = 0.00001f;
        float dot = start.Dot(end);

        float alpha;
        float beta;
        bool flip = false;
        if (dot < 0.0f)
        {
            dot *= -1.f;
            flip = true;
        }
        if (1.0f - dot > slerp_epsilon)
        {
            float theta = acos(dot);
            float sin_theta = sin(theta);
            alpha = sin((1.f - t) * theta) / sin_theta;
            beta = sin(t * theta) / sin_theta;
        }
        else
        {
            alpha = 1.f - t;
            beta = t;
        }

        if (flip) beta = -beta;
        r = alpha * start + beta * end;
        return r;
    }

    struct Vqs
    {
        DirectX::SimpleMath::Vector3 v;
        Quaternion q;
        float s;

        Vqs()
        {
            v = DirectX::SimpleMath::Vector3(0, 0, 0);
            q = Quaternion::Identity;
            s = 1.0f;
        }

        // do not use
        Vqs Inverse() const
        {
            Vqs result;
            Quaternion q_inverse = q.Inverse();
            result.s = (1.0f / s);
            result.q = q_inverse;
            result.v = Rotate(q_inverse, s * v);
            return result;
        }

        DirectX::SimpleMath::Matrix ToMatrix()
        {
            DirectX::SimpleMath::Matrix r = q.ToMatrix();
            // translation
            r._41 = v.x;
            r._42 = v.y;
            r._43 = v.z;

            // scale
            r._11 *= s;
            r._22 *= s;
            r._33 *= s;

            return r;
        }
    };

    // apply a rhs to the translation of lhs
    // concatenate rhs quaternion to lhs
    // scale lhs scale with rhs scale
    inline Vqs operator*(const Vqs& lhs, const Vqs& rhs)
    {
        Vqs result;
        result.v = Rotate(lhs.q, rhs.v) * lhs.s + lhs.v;
        result.q = rhs.q * lhs.q;
        result.s = rhs.s * lhs.s;
        return result;
    }

    inline float Clamp(float floor, float ceiling, float val)
    {
        float r = val;
        if (r < floor)   r = floor;
        if (r > ceiling) r = ceiling;
        return r;
    }
}
