#include "Math.hpp"
#include "../csgostructs.hpp"
namespace Math
{
    void MovementFix(CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles) {
        if (old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll) {
            Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

            auto viewangles = old_angles;
            auto movedata = Vector(m_Cmd->forwardmove, m_Cmd->sidemove, m_Cmd->upmove);
            viewangles.Normalize();

            if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && viewangles.roll != 0.f)
                movedata.y = 0.f;

            AngleVectors(wish_angle, wish_forward, wish_right, wish_up);
            AngleVectors(viewangles, cmd_forward, cmd_right, cmd_up);

            auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

            Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
                wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
                wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

            auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

            Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
                cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
                cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

            auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

            Vector correct_movement;
            correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) + (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
            correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) + (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
            correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) + (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

            correct_movement.x = Math::clamp(correct_movement.x, -450.f, 450.f);
            correct_movement.y = Math::clamp(correct_movement.y, -450.f, 450.f);
            correct_movement.z = Math::clamp(correct_movement.z, -320.f, 320.f);

            m_Cmd->forwardmove = correct_movement.x;
            m_Cmd->sidemove = correct_movement.y;
            m_Cmd->upmove = correct_movement.z;

            m_Cmd->buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
            if (m_Cmd->sidemove != 0.0) {
                if (m_Cmd->sidemove <= 0.0)
                    m_Cmd->buttons |= IN_MOVELEFT;
                else
                    m_Cmd->buttons |= IN_MOVERIGHT;
            }

            if (m_Cmd->forwardmove != 0.0) {
                if (m_Cmd->forwardmove <= 0.0)
                    m_Cmd->buttons |= IN_BACK;
                else
                    m_Cmd->buttons |= IN_FORWARD;
            }
        }
    }

    float RandomFloat(float min, float max)
    {
        static auto ranFloat = reinterpret_cast<float(*)(float, float)>(GetProcAddress(GetModuleHandleW(L"vstdlib.dll"), "RandomFloat"));
        if (ranFloat)
        {
            return ranFloat(min, max);
        }
        else
        {
            return 0.f;
        }
    }
	//--------------------------------------------------------------------------------
	float VectorDistance(const Vector& v1, const Vector& v2)
	{
		return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
	}
	//--------------------------------------------------------------------------------
	QAngle CalcAngle(const Vector& src, const Vector& dst)
	{
		QAngle vAngle;
		Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
		double hyp = sqrt(delta.x*delta.x + delta.y*delta.y);

		vAngle.pitch = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
		vAngle.yaw = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
		vAngle.roll = 0.0f;

		if (delta.x >= 0.0)
			vAngle.yaw += 180.0f;

		return vAngle;
	}
	//--------------------------------------------------------------------------------
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle)
	{
		Vector ang, aim;

		AngleVectors(viewAngle, aim);
		AngleVectors(aimAngle, ang);

		auto res = RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
		if (std::isnan(res))
			res = 0.f;
		return res;
	}
    //--------------------------------------------------------------------------------
    void ClampAngles(QAngle& angles)
    {
        if(angles.pitch > 89.0f) angles.pitch = 89.0f;
        else if(angles.pitch < -89.0f) angles.pitch = -89.0f;

        if(angles.yaw > 180.0f) angles.yaw = 180.0f;
        else if(angles.yaw < -180.0f) angles.yaw = -180.0f;

        angles.roll = 0;
    }
    //--------------------------------------------------------------------------------
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out)
    {
        out[0] = in1.Dot(in2[0]) + in2[0][3];
        out[1] = in1.Dot(in2[1]) + in2[1][3];
        out[2] = in1.Dot(in2[2]) + in2[2][3];
    }
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle &angles, Vector& forward)
    {
        float	sp, sy, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));

        forward.x = cp*cy;
        forward.y = cp*sy;
        forward.z = -sp;
    }
    //--------------------------------------------------------------------------------
    void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
    {
        float sr, sp, sy, cr, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));
        DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angles[2]));

        forward.x = (cp * cy);
        forward.y = (cp * sy);
        forward.z = (-sp);
        right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
        right.z = (-1 * sr * cp);
        up.x = (cr * sp * cy + -sr*-sy);
        up.y = (cr * sp * sy + -sr*cy);
        up.z = (cr * cp);
    }
    //--------------------------------------------------------------------------------
    void VectorAngles(const Vector& forward, QAngle& angles)
    {
        float	tmp, yaw, pitch;

        if(forward[1] == 0 && forward[0] == 0) {
            yaw = 0;
            if(forward[2] > 0)
                pitch = 270;
            else
                pitch = 90;
        } else {
            yaw = (atan2(forward[1], forward[0]) * 180 / DirectX::XM_PI);
            if(yaw < 0)
                yaw += 360;

            tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            pitch = (atan2(-forward[2], tmp) * 180 / DirectX::XM_PI);
            if(pitch < 0)
                pitch += 360;
        }

        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0;
    }
    //--------------------------------------------------------------------------------
    void VectorVectors(const Vector& forward, Vector& right, Vector& up) 
    {
        if (fabs(forward.x) < 1e-6 && fabs(forward.y) < 1e-6)
        {
            // pitch 90 degrees up/down from identity.
            right = Vector(0, -1, 0);
            up = Vector(-forward.z, 0, 0);
        }
        else
        {
            // get directions vector using cross product.
            right = forward.CrossProductVec(Vector(0, 0, 1)).Normalized();
            up = right.CrossProductVec(forward).Normalized();
        }
    }
    //--------------------------------------------------------------------------------
    static bool screen_transform(const Vector& in, Vector& out)
    {
        static auto& w2sMatrix = g_EngineClient->WorldToScreenMatrix();

        out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
        out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
        out.z = 0.0f;

        float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

        if(w < 0.001f) {
            out.x *= 100000;
            out.y *= 100000;
            return false;
        }

        out.x /= w;
        out.y /= w;

        return true;
    }
    //--------------------------------------------------------------------------------
    bool WorldToScreen(const Vector& in, Vector& out)
    {
        if(screen_transform(in, out)) {
            int w, h;
            g_EngineClient->GetScreenSize(w, h);

            out.x = (w / 2.0f) + (out.x * w) / 2.0f;
            out.y = (h / 2.0f) - (out.y * h) / 2.0f;

            return true;
        }
        return false;
    }
    float NormalizeYaw(float value)
    {
        while (value > 180)
            value -= 360.f;

        while (value < -180)
            value += 360.f;
        return value;
    }
    //--------------------------------------------------------------------------------
    Vector CrsProduct(const Vector& a, const Vector& b)
    {
        return Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }
    //--------------------------------------------------------------------------------
    void AngVec(const Vector& angles, Vector& forward)
    {
        float sp, sy, cp, cy;

        sy = sin(DEG2RAD(angles[1]));
        cy = cos(DEG2RAD(angles[1]));

        sp = sin(DEG2RAD(angles[0]));
        cp = cos(DEG2RAD(angles[0]));

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;
    }
    //--------------------------------------------------------------------------------
    void AngVec(const Vector& angles, Vector* forward, Vector* right, Vector* up)
    {
        auto sin_cos = [](float radian, float* sin, float* cos)
        {
            *sin = std::sin(radian);
            *cos = std::cos(radian);
        };

        float sp, sy, sr, cp, cy, cr;

        sin_cos(M_PI / 180.0f * angles.x, &sp, &cp);
        sin_cos(M_PI / 180.0f * angles.y, &sy, &cy);
        sin_cos(M_PI / 180.0f * angles.z, &sr, &cr);

        if (forward)
        {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }

        if (right)
        {
            right->x = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
            right->y = -1.0f * sr * sp * sy + -1.0f * cr * cy;
            right->z = -1.0f * sr * cp;
        }

        if (up)
        {
            up->x = cr * sp * cy + -sr * -sy;
            up->y = cr * sp * sy + -sr * cy;
            up->z = cr * cp;
        }
    }
    //--------------------------------------------------------------------------------
    void VecAng(const Vector& forward, Vector& angles)
    {
        Vector view;

        if (!forward[0] && !forward[1])
        {
            view[0] = 0.0f;
            view[1] = 0.0f;
        }
        else
        {
            view[1] = atan2(forward[1], forward[0]) * 180.0f / M_PI;

            if (view[1] < 0.0f)
                view[1] += 360.0f;

            view[2] = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            view[0] = atan2(forward[2], view[2]) * 180.0f / M_PI;
        }

        angles[0] = -view[0];
        angles[1] = view[1];
        angles[2] = 0.f;
    }
    //--------------------------------------------------------------------------------
    void VecAng(const Vector& forward, Vector& up, Vector& angles)
    {
        auto left = CrsProduct(up, forward);
        left.NormalizeInPlace();

        auto forwardDist = forward.Length2D();

        if (forwardDist > 0.001f)
        {
            angles.x = atan2(-forward.z, forwardDist) * 180.0f / M_PI;
            angles.y = atan2(forward.y, forward.x) * 180.0f / M_PI;

            auto upZ = (left.y * forward.x) - (left.x * forward.y);
            angles.z = atan2(left.z, upZ) * 180.0f / M_PI;
        }
        else
        {
            angles.x = atan2(-forward.z, forwardDist) * 180.0f / M_PI;
            angles.y = atan2(-left.x, left.y) * 180.0f / M_PI;
            angles.z = 0.0f;
        }
    }
    //--------------------------------------------------------------------------------
    Vector CalcAng(Vector src, Vector dst) {
        Vector angles;

        Vector delta = src - dst;
        float hyp = delta.Length2D();

        angles.y = std::atanf(delta.y / delta.x) * 57.2957795131f;
        angles.x = std::atanf(-delta.z / hyp) * -57.2957795131f;
        angles.z = 0.0f;

        if (delta.x >= 0.0f)
            angles.y += 180.0f;

        return angles;
    }
    //--------------------------------------------------------------------------------
    Vector VecAngleH(const Vector& forward) {
        Vector angles;
        float tmp, yaw, pitch;

        if (forward[1] == 0 && forward[0] == 0) {
            yaw = 0;
            if (forward[2] > 0)
                pitch = 270;
            else
                pitch = 90;
        }
        else {
            yaw = (atan2(forward[1], forward[0]) * 180 / DirectX::XM_PI);

            if (yaw < 0)
                yaw += 360;

            tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            pitch = (atan2(-forward[2], tmp) * 180 / DirectX::XM_PI);

            if (pitch < 0)
                pitch += 360;
        }

        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0;
        return angles;
    }
    //--------------------------------------------------------------------------------
    float RandFloat(float min, float max) {
        typedef float(*RandomFloat_t)(float, float);
        static RandomFloat_t m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat");
        return m_RandomFloat(min, max);
    }
    //--------------------------------------------------------------------------------
    void RandSeed(int seed) {
        typedef void(*RandomSeed_t)(int);
        static RandomSeed_t m_RandomSeed = (RandomSeed_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomSeed");
        return m_RandomSeed(seed);
    }
    //--------------------------------------------------------------------------------
    Vector IAngleTVectorI(const Vector& angles) {
        float sp, sy, cp, cy;

        DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
        DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));

        Vector forward;
        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;
        return forward;
    }
    //--------------------------------------------------------------------------------
    void clamp_angles(Vector& angles) {
        if (angles.x > 89.0f) angles.x = 89.0f;
        else if (angles.x < -89.0f) angles.x = -89.0f;

        if (angles.y > 180.0f) angles.y = 180.0f;
        else if (angles.y < -180.0f) angles.y = -180.0f;

        angles.z = 0;
    }
    //--------------------------------------------------------------------------------
    float Get_fov(Vector view_angle, Vector aim_angle) {
        CONST FLOAT MaxDegrees = 360.0f;
        Vector Delta(0, 0, 0);
        Vector Forward(0, 0, 0);
        Vector delta = aim_angle - view_angle;
        clamp_angles(delta);
        return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
    }
    //--------------------------------------------------------------------------------
    void AngleVectorsZ(const Vector& angles, Vector* forward)
    {
        float sp, sy, cp, cy;

        sy = sin(DEG2RAD(angles[1]));
        cy = cos(DEG2RAD(angles[1]));

        sp = sin(DEG2RAD(angles[0]));
        cp = cos(DEG2RAD(angles[0]));

        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }
    //--------------------------------------------------------------------------------
    void NormalizeAngles(QAngle& angles)
    {
        while (angles.pitch > 89.0f)
            angles.pitch -= 180.0f;

        while (angles.pitch < -89.0f)
            angles.pitch += 180.0f;

        while (angles.yaw < -180.0f)
            angles.yaw += 360.0f;

        while (angles.yaw > 180.0f)
            angles.yaw -= 360.0f;

        angles.roll = 0.0f;
    }
    //--------------------------------------------------------------------------------
    float ProductD(const float* v1, const float* v2)
    {
        return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
    };
    //--------------------------------------------------------------------------------
    void VecTrans(const float* in1, const matrix3x4_t& in2, float* out)
    {
        out[0] = ProductD(in1, in2[0]) + in2[0][3];
        out[1] = ProductD(in1, in2[1]) + in2[1][3];
        out[2] = ProductD(in1, in2[2]) + in2[2][3];
    };
    //--------------------------------------------------------------------------------
    void VecTransWrap(const Vector& in1, const matrix3x4_t& in2, Vector& out)
    {
        VecTrans(&in1.x, in2, &out.x);
    };
    //--------------------------------------------------------------------------------
    Vector vector_rotate(Vector& in1, matrix3x4_t& in2)
    {
        return Vector(in1.Dot(in2[0]), in1.Dot(in2[1]), in1.Dot(in2[2]));
    }
    //--------------------------------------------------------------------------------
    float DotProduct22(const float* v1, const float* v2) {
        return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
    }
    //--------------------------------------------------------------------------------
    void VectorRotate(const float* in1, const  matrix3x4_t& in2, float* out)
    {
        out[0] = DotProduct22(in1, in2[0]);
        out[1] = DotProduct22(in1, in2[1]);
        out[2] = DotProduct22(in1, in2[2]);
    }
    //--------------------------------------------------------------------------------
    void VectorRotate(const Vector& in1, const  matrix3x4_t& in2, Vector& out)
    {
        VectorRotate(&in1.x, in2, &out.x);
    }
    //--------------------------------------------------------------------------------
    void VectorRotate(const Vector& in1, const Vector& in2, Vector& out)
    {
        matrix3x4_t matRotate;
        AngleMatrix(in2, matRotate);
        VectorRotate(in1, matRotate, out);
    }
    //--------------------------------------------------------------------------------
    Vector vector_rotate(Vector& in1, Vector& in2)
    {
        matrix3x4_t m;
        AngleMatrix(in2, m);
        return vector_rotate(in1, m);
    }
    //--------------------------------------------------------------------------------
    void vector_i_rotate(Vector in1, matrix3x4_t in2, Vector& out)
    {
        out.x = in1.x * in2[0][0] + in1.y * in2[1][0] + in1.z * in2[2][0];
        out.y = in1.x * in2[0][1] + in1.y * in2[1][1] + in1.z * in2[2][1];
        out.z = in1.x * in2[0][2] + in1.y * in2[1][2] + in1.z * in2[2][2];
    }
    //--------------------------------------------------------------------------------
    void sinCos(float radians, PFLOAT sine, PFLOAT cosine)
    {
        __asm
        {
            fld dword ptr[radians]
            fsincos
            mov edx, dword ptr[cosine]
            mov eax, dword ptr[sine]
            fstp dword ptr[edx]
            fstp dword ptr[eax]
        }
    }
    //--------------------------------------------------------------------------------
    float DistanceToRay(const Vector& pos, const Vector& rayStart, const Vector& rayEnd, float* along, Vector* pointOnRay)
    {
        Vector to = pos - rayStart;
        Vector dir = rayEnd - rayStart;
        float length = dir.Normalize();

        float rangeAlong = dir.Dot(to);
        if (along)
            *along = rangeAlong;

        float range;

        if (rangeAlong < 0.0f)
        {
            range = -(pos - rayStart).Length();

            if (pointOnRay)
                *pointOnRay = rayStart;
        }
        else if (rangeAlong > length)
        {
            range = -(pos - rayEnd).Length();

            if (pointOnRay)
                *pointOnRay = rayEnd;
        }
        else
        {
            Vector onRay = rayStart + dir * rangeAlong;

            range = (pos - onRay).Length();

            if (pointOnRay)
                *pointOnRay = onRay;
        }

        return range;
    }
    //--------------------------------------------------------------------------------
    void AngleMatrix(const Vector& angles, matrix3x4_t& matrix)
    {
        float sr, sp, sy, cr, cp, cy;

        sinCos(DEG2RAD(angles[1]), &sy, &cy);
        sinCos(DEG2RAD(angles[0]), &sp, &cp);
        sinCos(DEG2RAD(angles[2]), &sr, &cr);

        matrix[0][0] = cp * cy;
        matrix[1][0] = cp * sy;
        matrix[2][0] = -sp;

        float crcy = cr * cy;
        float crsy = cr * sy;
        float srcy = sr * cy;
        float srsy = sr * sy;
        matrix[0][1] = sp * srcy - crsy;
        matrix[1][1] = sp * srsy + crcy;
        matrix[2][1] = sr * cp;

        matrix[0][2] = (sp * crcy + srsy);
        matrix[1][2] = (sp * crsy - srcy);
        matrix[2][2] = cr * cp;

        matrix[0][3] = 0.0f;
        matrix[1][3] = 0.0f;
        matrix[2][3] = 0.0f;
    }
    //--------------------------------------------------------------------------------
    void MatrixCopy(const matrix3x4_t& in, matrix3x4_t& out) 
    {
        std::memcpy(out.Base(), in.Base(), sizeof(matrix3x4_t));
    }
    //--------------------------------------------------------------------------------
    void ConcatTransforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out) 
    {
        if (&in1 == &out) 
        {
            matrix3x4_t in1b;
            MatrixCopy(in1, in1b);
            ConcatTransforms(in1b, in2, out);
            return;
        }

        if (&in2 == &out) 
        {
            matrix3x4_t in2b;
            MatrixCopy(in2, in2b);
            ConcatTransforms(in1, in2b, out);
            return;
        }

        out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
        out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
        out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
        out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];

        out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
        out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
        out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
        out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];

        out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
        out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
        out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
        out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
    }
    //--------------------------------------------------------------------------------
    bool intersect_line_with_bb(Vector& start, Vector& end, Vector& min, Vector& max) {
        float d1, d2, f;
        auto start_solid = true;
        auto t1 = -1.0f, t2 = 1.0f;

        const float s[3] = { start.x, start.y, start.z };
        const float e[3] = { end.x, end.y, end.z };
        const float mi[3] = { min.x, min.y, min.z };
        const float ma[3] = { max.x, max.y, max.z };

        for (auto i = 0; i < 6; i++) {
            if (i >= 3) {
                const auto j = i - 3;

                d1 = s[j] - ma[j];
                d2 = d1 + e[j];
            }
            else {
                d1 = -s[i] + mi[i];
                d2 = d1 - e[i];
            }

            if (d1 > 0.0f && d2 > 0.0f)
                return false;

            if (d1 <= 0.0f && d2 <= 0.0f)
                continue;

            if (d1 > 0)
                start_solid = false;

            if (d1 > d2) {
                f = d1;
                if (f < 0.0f)
                    f = 0.0f;

                f /= d1 - d2;
                if (f > t1)
                    t1 = f;
            }
            else {
                f = d1 / (d1 - d2);
                if (f < t2)
                    t2 = f;
            }
        }

        return start_solid || (t1 < t2&& t1 >= 0.0f);
    }
    //--------------------------------------------------------------------------------
    void vector_i_transform(const Vector& in1, const matrix3x4_t& in2, Vector& out)
    {
        out.x = (in1.x - in2[0][3]) * in2[0][0] + (in1.y - in2[1][3]) * in2[1][0] + (in1.z - in2[2][3]) * in2[2][0];
        out.y = (in1.x - in2[0][3]) * in2[0][1] + (in1.y - in2[1][3]) * in2[1][1] + (in1.z - in2[2][3]) * in2[2][1];
        out.z = (in1.x - in2[0][3]) * in2[0][2] + (in1.y - in2[1][3]) * in2[1][2] + (in1.z - in2[2][3]) * in2[2][2];
    }
    //--------------------------------------------------------------------------------
    void AngleToVec( const Vector& angles, Vector& forward )
    {
        Assert( s_bMathlibInitialized );
        Assert( forward );

        float sp, sy, cp, cy;

        sy = sin( DEG2RAD( angles.y ) );
        cy = cos( DEG2RAD( angles.y ) );

        sp = sin( DEG2RAD( angles.x ) );
        cp = cos( DEG2RAD( angles.x ) );

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;
    }
    //--------------------------------------------------------------------------------
    void VectorToAng( const Vector& forward, Vector& angles )
    {
        float tmp, yaw, pitch;

        if ( forward.y == 0 && forward.x == 0 )
        {
            yaw = 0;
            if ( forward.z > 0 )
                pitch = 270;
            else
                pitch = 90;
        }
        else
        {
            yaw = (atan2( forward.y, forward.x ) * 180 / M_PI);
            if ( yaw < 0 )
                yaw += 360;

            tmp = sqrt( forward.x * forward.x + forward.y * forward.y );
            pitch = (atan2( -forward.z, tmp ) * 180 / M_PI);
            if ( pitch < 0 )
                pitch += 360;
        }

        angles.x = pitch;
        angles.y = yaw;
        angles.z = 0;
    }
    //--------------------------------------------------------------------------------
    void AngleVectorMulti( const Vector angles, Vector& forward, Vector& right, Vector& up )
    {
        float angle;
        static float sp, sy, cp, cy;

        angle = angles.x * (M_PI / 180.f);
        sp = sin( angle );
        cp = cos( angle );

        angle = angles.y * (M_PI / 180.f);
        sy = sin( angle );
        cy = cos( angle );

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;

        static float sr, cr;

        angle = angles.z * (M_PI / 180.f);
        sr = sin( angle );
        cr = cos( angle );

        right.x = -1 * sr * sp * cy + -1 * cr * -sy;
        right.y = -1 * sr * sp * sy + -1 * cr * cy;
        right.z = -1 * sr * cp;

        up.x = cr * sp * cy + -sr * -sy;
        up.y = cr * sp * sy + -sr * cy;
        up.z = cr * cp;
    }

}
