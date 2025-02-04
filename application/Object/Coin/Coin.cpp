#include "Coin.h"

#include "math/Easing.h"

static float ModulationEasing(float t, float modulationAmplitude, float modulationPeriod) {
	return modulationAmplitude * (0.5f - 0.5f * std::cos(2.0f * 3.14f * t / modulationPeriod));
}

void Coin::Init(const std::string className) 
{ 
	BaseObject::Init(className);
	Collider::SetVisible(false);
}

void Coin::Update() 
{ 
	BaseObject::Update();

	// タイマーの加算
	globalTime_ += kDeltaTime;

	float baseAngle = rotationSpeed_ * globalTime_;

	float modulation = ModulationEasing(std::fmod(globalTime_, modulationPeriod_), modulationAmplitude_, modulationPeriod_);

	float totalAngle = baseAngle + modulation;

	// 度をラジアンに変換して現在のY軸に適用
	float angleRadians = totalAngle * (3.14f / 180.0f);
	BaseObject::SetRotation({0.0f, angleRadians, 0.0f});
}

void Coin::Draw(const ViewProjection& viewProjection) 
{ 
	BaseObject::Draw(viewProjection); 
}
