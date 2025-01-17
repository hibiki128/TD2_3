#pragma once
#include"Input.h"
#include"Sprite.h"

class Pause
{
public:
	/// ===================================================
	/// public method
	/// ===================================================
	
	void Init();
	void Update();
	void Draw();

	bool IsPause() { return isPause_; };

private:
	/// ===================================================
	/// private method
	/// ===================================================

	void OpenMenu();

	void MenuOperation();

private:
	/// ===================================================
	/// private variaus
	/// ===================================================

	Input* input_ = nullptr; 

	std::unique_ptr<Sprite> backGround_;

	Vector4 color_ = { 1.0f,1.0f,1.0f,0.0f }; // ポーズ中の背景の色

	Vector2 spritePosition_ = { 0.0f,0.0f };  // ポーズ中の背景の位置

	int currentItem_= 0;             // 現在選択しているメニュー項目

	bool isPause_ = false;           // ポーズしてるかどうか
	bool CanEscape_ = false;          // エスケープのクールタイム用

	float startAlpha_ = 0.0f;
	float endAlpha_ = 0.0f;
	float alphaT_ = 0.0f;
	float EscapeCoolTime_ = 0.0f;

	const float easeTMax_ = 0.2f;
};

