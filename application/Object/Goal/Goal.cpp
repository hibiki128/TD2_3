#include "Goal.h"

void Goal::Init(const std::string className)
{
	BaseObject::Init(className);
	Collider::SetVisible(false);
}

void Goal::Update()
{
	BaseObject::Update();
}

void Goal::Draw(const ViewProjection &viewProjection, Vector3 offSet ) {
	BaseObject::Draw(viewProjection);
}
