#pragma once
#include"Sprite.h"
#include"application/Base/BaseObject.h"
#include"memory"
class TitleUI
{
public:
	void Init();
	void Update();
	void Draw(const ViewProjection& vp);
	void Debug();
private:
	std::unique_ptr<BaseObject> start_;
	std::unique_ptr<BaseObject> title_;
	std::unique_ptr<Sprite> buttonA_;

	Vector2 buttonA_pos;

	bool isStart_ = false;
};

