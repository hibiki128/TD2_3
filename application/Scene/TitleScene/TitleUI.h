#pragma once
#include"Sprite.h"
#include"application/Base/BaseObject.h"
#include"memory"
#include <Input.h>
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
	//std::unique_ptr<BaseObject> start_;
	std::unique_ptr<BaseObject> title_;
	float t_;
	float startTime_ = 1.0f;

	// 入力
	Input* input_;

	///
	/// オブジェクト
	///
	std::unique_ptr<BaseObject> objectBook_; // 本
	std::unique_ptr<BaseObject> objectUI_;  // 栞

	std::unique_ptr<BaseObject> objectL_; // Lスティック
	std::unique_ptr<BaseObject> objectA_; // Aボタン
	std::unique_ptr<BaseObject> objectR_; // RBボタン
private:
	// Lスティックオブジェクトの初期位置
	Vector3 initLstickPos_;

	// パッド入力による反応
	void InputReaction();
};

