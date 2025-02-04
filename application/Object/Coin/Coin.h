#pragma once

// Application
#include "application/Base/BaseObject.h"

class Coin : public BaseObject
{
public:
	void Init(const std::string className) override;
	void Update() override;
	void Draw(const ViewProjection& viewProjection) override;

	void SetCollected(bool flag) { isCollected_ = flag; }
	bool IsCollected() const { return isCollected_; }

private:
	bool isCollected_ = false; // プレイヤーに取得されたかどうか


	const float kDeltaTime = 1.0f / 60.0f;
	float rotationSpeed_ = 280.0f; // 1秒間に回転する角度

	float globalTime_ = 0.0f;
	float modulationAmplitude_ = 20.0f;
	float modulationPeriod_ = 2.0f; 

};
