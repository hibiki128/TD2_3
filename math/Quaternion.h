#pragma once
#include "Vector3.h"
#include <cmath>

class Quaternion {
public:
    float w, x, y, z;

    // コンストラクタ
    Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    // 2つのベクトルの間の回転を計算
    void SetFromTo(const Vector3& from, const Vector3& to);

    // クォータニオンをオイラー角に変換
    Vector3 ToEulerAngles() const;
    void SetFromEulerAngles(const Vector3& eulerAngles);
    // クォータニオンの長さ
    float Length() const;
    // クォータニオンを正規化
    Quaternion Normalize() const;
};