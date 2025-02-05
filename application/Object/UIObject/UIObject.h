#pragma once

#include "application/Base/BaseObject.h"
#include "Sprite.h"

// Engine
#include "Input.h"

class UIObject 
{
public:
	void Init(int currentStageNum);
	void Update();
	void Draw(const ViewProjection& viewProjection);
	void DrawSprite();
	void DebugImGui();

private:
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

	///
	///	スプライト
	/// 
	std::unique_ptr<Sprite> spritePause_; // ポーズボタン

private:
	// Lスティックオブジェクトの初期位置
	Vector3 initLstickPos_;

	// パッド入力による反応
	void InputReaction();

private:
	// 現在の選択ステージ
	int currentStageNum_ = -1;
	// 選択されたステージによってUIの位置を調整する
	void AdjustUIPositionForStageNum();
};
