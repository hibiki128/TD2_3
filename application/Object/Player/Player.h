#pragma once
#include "application/Base/BaseObject.h"

class Player : public BaseObject
{
public:
	void Init(const std::string className)override;
	void Update()override;
	void Draw(const ViewProjection& viewProjection)override;
};

