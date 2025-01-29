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
	void SetStageNum(const int& stageNum) { stageNum_ = stageNum; }
private:
	void MenuOperation();
	void InitNumbers();
	void SetNumber();
private:
	Input* input_ = nullptr;
	std::unique_ptr<BaseObject> book_;
	std::unique_ptr<BaseObject> stage_;
	std::unique_ptr<BaseObject> nextStage_;
	std::unique_ptr<BaseObject> backSelect_;
	std::unique_ptr<BaseObject> restart_;
	std::unique_ptr<BaseObject> singleDigit_;
	std::unique_ptr<BaseObject> twoDigit_;

	std::vector<std::unique_ptr<Object3d>> numbers_;


	int currentItem_;
	int stageNum_;
	float coolTime_;
};

