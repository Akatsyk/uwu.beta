#pragma once

#include "../sdk.hpp"

#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)

#define M_PI 3.14159265358979323846
#define PI_F	((float)(M_PI)) 

#define Assert( _exp ) ((void)0)

namespace Math
{
	void MovementFix(CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles);
	float RandomFloat(float min, float max);
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)&x;

		i += 127 << 23;
		i >>= 1;
		return *(float*)&i;
	}
	float VectorDistance(const Vector& v1, const Vector& v2);
	QAngle CalcAngle(const Vector& src, const Vector& dst);
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle);
	template<class T>
	void Normalize3(T& vec)
	{
		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] >  180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}
    void ClampAngles(QAngle& angles);
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
    void AngleVectors(const QAngle &angles, Vector& forward);
    void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up);
    void VectorAngles(const Vector& forward, QAngle& angles);
	void VectorVectors(const Vector& forward, Vector& right, Vector& up);
    bool WorldToScreen(const Vector& in, Vector& out);
	float NormalizeYaw(float value);
	void NormalizeAngles(QAngle& angles);
	template<class T, class U>
	static T clamp(const T& in, const U& low, const U& high)
	{
		if (in <= low)
			return low;

		if (in >= high)
			return high;

		return in;
	}
	//--------------------------------------------------------------------------------
	Vector CrsProduct(const Vector& a, const Vector& b);
	//--------------------------------------------------------------------------------
	void AngVec(const Vector& angles, Vector& forward);
	//--------------------------------------------------------------------------------
	void AngVec(const Vector& angles, Vector* forward, Vector* right, Vector* up);
	//--------------------------------------------------------------------------------
	void VecAng(const Vector& forward, Vector& angles);
	//--------------------------------------------------------------------------------
	void VecAng(const Vector& forward, Vector& up, Vector& angles);
	Vector CalcAng(Vector src, Vector dst);
	Vector VecAngleH(const Vector& forward);
	float RandFloat(float min, float max);
	void RandSeed(int seed);
	Vector IAngleTVectorI(const Vector& angles);
	float Get_fov(Vector view_angle, Vector aim_angle);
	void AngleVectorsZ(const Vector& angles, Vector* forward);
	float ProductD(const float* v1, const float* v2);
	void VecTrans(const float* in1, const matrix3x4_t& in2, float* out);
	void VecTransWrap(const Vector& in1, const matrix3x4_t& in2, Vector& out);

	Vector vector_rotate(Vector& in1, matrix3x4_t& in2)
		;

	float DotProduct22(const float* v1, const float* v2);

	void VectorRotate(const float* in1, const  matrix3x4_t& in2, float* out)
		;

	void VectorRotate(const Vector& in1, const  matrix3x4_t& in2, Vector& out)
		;

	void VectorRotate(const Vector& in1, const Vector& in2, Vector& out)
		;

	Vector vector_rotate(Vector& in1, Vector& in2);
	void vector_i_rotate(Vector in1, matrix3x4_t in2, Vector& out);
	void sinCos(float radians, PFLOAT sine, PFLOAT cosine);
    void AngleMatrix(const Vector& angles, matrix3x4_t& matrix);
	void MatrixCopy(const matrix3x4_t& in, matrix3x4_t& out);
	void ConcatTransforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out);
	bool intersect_line_with_bb(Vector& start, Vector& end, Vector& min, Vector& max);
	void vector_i_transform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	void AngleToVec( const Vector& angles, Vector& forward );
	void VectorToAng( const Vector& forward, Vector& angles );
	float DistanceToRay(const Vector& pos, const Vector& rayStart, const Vector& rayEnd, float* along = nullptr, Vector* pointOnRay = nullptr);
	void AngleVectorMulti( const Vector angles, Vector& forward, Vector& right, Vector& up );
}