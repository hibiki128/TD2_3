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
	void TextMove();
private:
	std::unique_ptr<BaseObject> start_;
	std::unique_ptr<BaseObject> title_;
	float t_;
	float startTime_ = 1.0f;
};

