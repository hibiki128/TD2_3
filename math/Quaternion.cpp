#include "Quaternion.h"
#include"numbers"

void Quaternion::SetFromTo(const Vector3& from, const Vector3& to)
{
    Vector3 f = from.Normalize(); // 正規化したfromベクトル
    Vector3 t = to.Normalize();   // 正規化したtoベクトル

    Vector3 cross = f.Cross(t);    // fromとtoのクロス積
    float dot = f.Dot(t);          // fromとtoの内積

    // 回転角をクォータニオンに変換
    w = sqrt((1.0f + dot) * 0.5f); // 回転のスカラー成分
    float s = 0.5f / w;

    x = cross.x * s;
    y = cross.y * s;
    z = cross.z * s;
}

Vector3 Quaternion::ToEulerAngles() const
{
    Vector3 angles;

    // ピッチ（X軸）
    float sinPitch = 2.0f * (w * x + y * z);
    float cosPitch = 1.0f - 2.0f * (x * x + y * y);
    angles.x = atan2(sinPitch, cosPitch);

    // ヨー（Y軸）
    float sinYaw = 2.0f * (w * y - z * x);
    angles.y = fabs(sinYaw) >= 1.0f ? copysign(std::numbers::pi_v<float> / 2, sinYaw) : asin(sinYaw); // 特別なケース

    // ロール（Z軸）
    float sinRoll = 2.0f * (w * z + x * y);
    float cosRoll = 1.0f - 2.0f * (y * y + z * z);
    angles.z = atan2(sinRoll, cosRoll);

    return angles;
}

void Quaternion::SetFromEulerAngles(const Vector3& eulerAngles)
{
    // ピッチ（X軸）、ヨー（Y軸）、ロール（Z軸）をラジアンで取得
    float halfPitch = eulerAngles.x * 0.5f;
    float halfYaw = eulerAngles.y * 0.5f;
    float halfRoll = eulerAngles.z * 0.5f;

    float sinPitch = std::sin(halfPitch);
    float cosPitch = std::cos(halfPitch);
    float sinYaw = std::sin(halfYaw);
    float cosYaw = std::cos(halfYaw);
    float sinRoll = std::sin(halfRoll);
    float cosRoll = std::cos(halfRoll);

    // クォータニオンを計算
    w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
    x = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
    y = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
}

float Quaternion::Length() const
{
    return std::sqrt(w * w + x * x + y * y + z * z);
}

Quaternion Quaternion::Normalize() const
{
    float len = Length();
    if (len == 0.0f) return Quaternion();
    return Quaternion(w / len, x / len, y / len, z / len);
}
