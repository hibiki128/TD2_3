#include "Enemy.h"

void Enemy::Init(const std::string className)
{
	BaseObject::Init(className);
	BaseObject::CreateModel("debug/Cube.obj");
	BaseObject::CreateCollider();
}

void Enemy::Update()
{
	BaseObject::Update();
}

void Enemy::Draw(const ViewProjection& viewProjection)
{
	BaseObject::Draw(viewProjection);
}
