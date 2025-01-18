#pragma once

// Application
#include "application/Base/BaseObject.h"

class Goal : public BaseObject
{
public:
	void Init(const std::string className) override;
	void Update() override;
	void Draw(const ViewProjection& viewProjection) override;
};