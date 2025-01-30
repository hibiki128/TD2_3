#pragma once
#include"application/Base/BaseObject.h"
#include <ParticleEmitter.h>
class Input;
class ClearUI
{
public:
	void Init();
	void Update();
	void Draw(const ViewProjection& vp);
	void DrawParticle(const ViewProjection& vp);
	void DrawTexts(const ViewProjection& vp);
	void Debug();
	int GetItemNum() { return currentItem_; }
	void SetStageNum(const int& stageNum) { stageNum_ = stageNum; }
	void SetDecision(const bool& dicision) { isDecision_ = dicision; }
private:
	void MenuOperation();
	void InitNumbers();
	void MoveUI();
private:
	Input* input_ = nullptr;
	std::unique_ptr<BaseObject> book_;
	std::unique_ptr<BaseObject> stage_;
	std::unique_ptr<BaseObject> nextStage_;
	std::unique_ptr<BaseObject> backSelect_;
	std::unique_ptr<BaseObject> retry_;
	std::unique_ptr<BaseObject> singleDigit_;
	std::unique_ptr<BaseObject> twoDigit_;
	std::unique_ptr<BaseObject> animaChara_;
	std::unique_ptr<ParticleEmitter> decisionEmitter_;

	int currentItem_;
	int stageNum_;

	float coolTime_;
	float nextT_;
	float retryT_;
	float selectT_;

	bool isDecision_ = false;
};

