#include "Quaternion.h"

namespace Framework
{
    const Quaternion Quaternion::Identity = Quaternion(0.f, 0.f, 0.f, 1.f);

    Quaternion::Quaternion(void)
        :
        s(1.0f),
        v(0.0f, 0.0f, 0.0f)
    {
    }

    Quaternion::Quaternion(float i, float j, float k, float s_)
        :
        s(s_),
        v(i, j, k)
    {
    }

    Quaternion::Quaternion(DirectX::SimpleMath::Vector3 v_, float s_)
        :
        s(s_),
        v(v_)
    {

    }

    Quaternion::~Quaternion(void)
    {
    }

    Quaternion Quaternion::CreateFromAxisAngle(const DirectX::SimpleMath::Vector3& axis, float angle)
    {
        Quaternion r;
        DirectX::SimpleMath::Vector3 a = axis;
        a.Normalize();

        float sinV, cosV;
        DirectX::XMScalarSinCos(&sinV, &cosV, 0.5f * angle);
        r.v = a * sinV;
        r.s = cosV;
        return r;
    }

    const float Quaternion::Dot(const Quaternion& rhs) const
    {
        return this->s * rhs.s + this->v.Dot(rhs.v);
    }

    const float Quaternion::LengthSquared() const
    {
        return this->s * this->s + this->v.Dot(this->v);
    }

    const float Quaternion::Length() const
    {
        return sqrt(this->LengthSquared());
    }

    void Quaternion::Normalize(void)
    {
        float l = 1.0f / this->Length();
        this->s *= l;
        this->v *= l;
    }

    Quaternion Quaternion::GetNormalizedCopy(void) const
    {
        float l = 1.0f / this->Length();
        return Quaternion(this->v * l, this->s * l);
    }

    void Quaternion::Conjugate(void)
    {
        this->v *= -1;
    }

    Quaternion Quaternion::GetConjugateCopy(void) const
    {
        return Quaternion(this->v * -1, this->s);
    }

    Quaternion Quaternion::Inverse(void) const
    {
        return this->GetConjugateCopy() * (1.0f / this->LengthSquared());
    }

    // assumes quaternion is unit length, otherwise resulting matrix is scaled
    DirectX::SimpleMath::Matrix Quaternion::ToMatrix(void) const
    {
        using namespace DirectX::SimpleMath;
        // from
        // www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/jay.htm
        Matrix m1 = Matrix(
            s, v.z, -v.y, v.x,
            -v.z, s, v.x, v.y,
            v.y, -v.x, s, v.z,
            -v.x, -v.y, -v.z, s
        );

        Matrix m2 = Matrix(
            s, v.z, -v.y, -v.x,
            -v.z, s, v.x, -v.y,
            v.y, -v.x, s, -v.z,
            v.x, v.y, v.z, s
        );

        return m2 * m1;
    }
}
