#pragma once
#include"application/Base/BaseObject.h"
class Input;
class ClearUI
{
public:
	void Init();
	void Update();
	void Draw(const ViewProjection& vp);
	void Debug();
	int GetItemNum() { return currentItem_; }
private:
	void MenuOperation();
private:
	Input* input_ = nullptr;
	std::unique_ptr<BaseObject> book_;

	int currentItem_;
	float coolTime_;
};

