#pragma once
#include"Sprite.h"
#include <memory>
class SelectUI
{
public:
	void Init();
	void Update();
	void Draw();

private:

	std::unique_ptr<Sprite> backTitle_;
	std::unique_ptr<Sprite> decision_;
	Vector2 pos_;
	Vector2 size_;
	Vector2 size2_;
	float value_;
};

