#include "Player.h"

void Player::Init(const std::string className)
{
	BaseObject::Init(className);
	BaseObject::CreateModel("debug/Cube.obj");
	BaseObject::CreateCollider();
}

void Player::Update()
{
	BaseObject::Update();
}

void Player::Draw(const ViewProjection& viewProjection)
{
	BaseObject::Draw(viewProjection);
}
