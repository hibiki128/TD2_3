#include "Block.h"

void Block::Init(const std::string className) { 
	BaseObject::Init(className);
	Collider::SetCollisionType(CollisionType::AABB);
	Collider::SetVisible(false);
}

void Block::Update() { BaseObject::Update(); }

void Block::Draw(const ViewProjection &viewProjection, Vector3 offSet) { BaseObject::Draw(viewProjection); }
