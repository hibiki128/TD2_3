#pragma once
#include"ViewProjection.h"
class ClearCamera
{
public:
	void Init(ViewProjection* viewProjection);
	void Update(Vector3 tragetPos);

	bool GetActive() { return isActive_; }
	bool GetFinish() { return isFinish_; }
	void SetActive(bool isActive) { isActive_ = isActive; }
private:
	ViewProjection* viewProjection_;

	// X,Y,Z軸回りのローカル回転角
	Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };
	// ローカル座標
	Vector3 translation_ = { 0.0f, 0.0f, -50.0f };
	Matrix4x4 matRot_;
	
	bool isActive_ = false;
	bool isFinish_ = false;

	float t = 0.0f;
};

