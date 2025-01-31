#include "Coin.h"

void Coin::Init(const std::string className) 
{ 
	BaseObject::Init(className);
	Collider::SetVisible(false);
}

void Coin::Update() 
{ 
	BaseObject::Update();
}

void Coin::Draw(const ViewProjection& viewProjection) 
{ 
	BaseObject::Draw(viewProjection); 
}
