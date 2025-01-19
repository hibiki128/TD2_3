#pragma once
#include"Input.h"
#include"Sprite.h"
#include"application/Base/BaseObject.h"

class Pause
{
public:
	/// ===================================================
	/// public method
	/// ===================================================

	void Init();
	void Update();
	void Draw(const ViewProjection& vp);

	bool IsPause() { return isPause_; };

private:
	/// ===================================================
	/// private method
	/// ===================================================

	void InitText();

	void UpdateText();

	void MoveText();

	void OpenMenu();

	void MenuOperation();

	void Debug();

private:
	/// ===================================================
	/// private variaus
	/// ===================================================

	Input* input_ = nullptr;

	std::unique_ptr<Sprite> backGround_;

	Vector4 color_ = { 1.0f,1.0f,1.0f,0.0f }; // ポーズ中の背景の色

	Vector2 backGamePos = { 0.0f,0.0f };
	Vector2 backSelectPos = { 0.0f,0.0f };
	Vector2 restartPos = { 0.0f,0.0f };
	Vector2 spritePosition_ = { 0.0f,0.0f };  // ポーズ中の背景の位置
	Vector2 backGameSize;
	Vector2 backSelectSize;
	Vector2 restartSize;

	int currentItem_ = 0;                     // 現在選択しているメニュー項目

	bool isPause_ = false;                    // ポーズしてるかどうか
	bool CanEscape_ = false;                  // エスケープのクールタイム用
	bool previousIsPause_ = false;            // クラスメンバに前フレームのポーズ状態を保持する変数を追加
	bool textMovedRight_ = false;             // 全部のテキストが右に行ったかどうか

	struct EasingValue {
		float start_;
		float end_;
		float T_;
		float TMax_;
	};

	EasingValue alpha_E;
	EasingValue backGame_E;
	EasingValue backSelect_E;
	EasingValue restart_E;

	float EscapeCoolTime_ = 0.0f;             // Escキーのクールタイム

	std::unique_ptr<Sprite> backGame_;
	std::unique_ptr<Sprite> backSelect_;
	std::unique_ptr<Sprite> Restart_;

};

