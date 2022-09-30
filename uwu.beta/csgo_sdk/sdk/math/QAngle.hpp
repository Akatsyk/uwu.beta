#pragma once

class QAngle
{
public:
    QAngle(void)
    {
        Init();
    }
    QAngle(float X, float Y, float Z)
    {
        Init(X, Y, Z);
    }
    QAngle(const float* clr)
    {
        Init(clr[0], clr[1], clr[2]);
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        pitch = ix;
        yaw = iy;
        roll = iz;
    }

    float operator[](int i) const
    {
        return ((float*)this)[i];
    }
    float& operator[](int i)
    {
        return ((float*)this)[i];
    }

    QAngle& operator+=(const QAngle& v)
    {
        pitch += v.pitch; yaw += v.yaw; roll += v.roll;
        return *this;
    }
    QAngle& operator-=(const QAngle& v)
    {
        pitch -= v.pitch; yaw -= v.yaw; roll -= v.roll;
        return *this;
    }
    QAngle& operator*=(float fl)
    {
        pitch *= fl;
        yaw *= fl;
        roll *= fl;
        return *this;
    }
    QAngle& operator*=(const QAngle& v)
    {
        pitch *= v.pitch;
        yaw *= v.yaw;
        roll *= v.roll;
        return *this;
    }
    QAngle& operator/=(const QAngle& v)
    {
        pitch /= v.pitch;
        yaw /= v.yaw;
        roll /= v.roll;
        return *this;
    }
    QAngle& operator+=(float fl)
    {
        pitch += fl;
        yaw += fl;
        roll += fl;
        return *this;
    }
    QAngle& operator/=(float fl)
    {
        pitch /= fl;
        yaw /= fl;
        roll /= fl;
        return *this;
    }
    QAngle& operator-=(float fl)
    {
        pitch -= fl;
        yaw -= fl;
        roll -= fl;
        return *this;
    }

    QAngle& operator=(const QAngle &vOther)
    {
        pitch = vOther.pitch; yaw = vOther.yaw; roll = vOther.roll;
        return *this;
    }

    QAngle operator-(void) const
    {
        return QAngle(-pitch, -yaw, -roll);
    }
    QAngle operator+(const QAngle& v) const
    {
        return QAngle(pitch + v.pitch, yaw + v.yaw, roll + v.roll);
    }
    QAngle operator-(const QAngle& v) const
    {
        return QAngle(pitch - v.pitch, yaw - v.yaw, roll - v.roll);
    }
    QAngle operator*(float fl) const
    {
        return QAngle(pitch * fl, yaw * fl, roll * fl);
    }
    QAngle operator*(const QAngle& v) const
    {
        return QAngle(pitch * v.pitch, yaw * v.yaw, roll * v.roll);
    }
    QAngle operator/(float fl) const
    {
        return QAngle(pitch / fl, yaw / fl, roll / fl);
    }
    QAngle operator/(const QAngle& v) const
    {
        return QAngle(pitch / v.pitch, yaw / v.yaw, roll / v.roll);
    }

    float Length() const
    {
        return sqrt(pitch*pitch + yaw*yaw + roll*roll);
    }
    float LengthSqr(void) const
    {
        return (pitch*pitch + yaw*yaw + roll*roll);
    }
    bool IsZero(float tolerance = 0.01f) const
    {
        return (pitch > -tolerance && pitch < tolerance &&
            yaw > -tolerance && yaw < tolerance &&
            roll > -tolerance && roll < tolerance);
    }

	float Normalize() const
	{
		QAngle res = *this;
		float l = res.Length();
		if (l != 0.0f)
		{
			res /= l;
		}
		else
		{
			res[0] = res[1] = res[2] = 0.0f;
		}
		return l;
	}

    __forceinline float Dot(const Vector& v) const {
        return (pitch * v.x + yaw * v.y + roll * v.z);
    }

    __forceinline float Dot(float* v) const {
        return (pitch * v[0] + yaw * v[1] + roll * v[2]);
    }

    __forceinline Vector CrossProductAng(const Vector& v) const {
        return {
            (yaw * v.z) - (roll * v.y),
            (roll * v.x) - (pitch * v.z),
            (pitch * v.y) - (yaw * v.x)
        };
    }

    __forceinline float DistTo(const Vector& v) const {
        Vector delta;

        delta.x = pitch - v.x;
        delta.y = yaw - v.y;
        delta.z = roll - v.z;

        return delta.Length2D();
    }

    __forceinline void ToVectors(Vector& right, Vector& up) {
        Vector tmp;
        if (pitch == 0.f && yaw == 0.f)
        {
            // pitch 90 degrees up/down from identity.
            right[0] = 0.f;
            right[1] = -1.f;
            right[2] = 0.f;
            up[0] = -roll;
            up[1] = 0.f;
            up[2] = 0.f;
        }
        else
        {
            tmp[0] = 0; tmp[1] = 0; tmp[2] = 1;

            // get directions vector using cross product.
            right = CrossProductAng(tmp).Normalized();
            up = right.CrossProductVec(Vector(pitch, yaw, roll)).Normalized();
        }
    }

    float pitch;
    float yaw;
    float roll;
};

inline QAngle operator*(float lhs, const QAngle& rhs)
{
    return rhs * lhs;
}
inline QAngle operator/(float lhs, const QAngle& rhs)
{
    return rhs / lhs;
}
