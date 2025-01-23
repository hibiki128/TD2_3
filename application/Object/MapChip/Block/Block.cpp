#include "Block.h"

void Block::Init(const std::string className) { 
	BaseObject::Init(className);
	Collider::SetCollisionType(CollisionType::AABB);
}

void Block::Update() { BaseObject::Update(); }

void Block::Draw(const ViewProjection& viewProjection) { BaseObject::Draw(viewProjection); }
