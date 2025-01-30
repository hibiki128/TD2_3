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
};
