#pragma once

// Engine
#include "Input.h"

// Application
#include "application/Base/BaseObject.h"
#include "application/Object/MapChip/MapChipField.h"
#include "myEngine/utility/collider/Collider.h"

class Player : public BaseObject
{
public:
	void Init(const std::string className)override;
	void Update(MapChipField* mapChipField);
	void Draw(const ViewProjection& viewProjection)override;
	void DebugImGui()override;

private:
	// 入力
	Input* input_;

	///
	/// 基本的なパラメータ
	/// 
	
	// サイズ
	const float kWidth = 2.0f;
	const float kHeight = 2.0f;

	// 速度
	Vector3 velocity_;

	// ジャンプ関連
	bool isOnGround_ = false; // 着地しているかフラグ
	bool isJumping_ = false; // ジャンプ中かフラグ
	float jumpVelocity_; // ジャンプ時の初速
	float gravity_; // 重力

private:
	// 移動関数
	void Move();
	// ジャンプ処理
	void Jump();
	// 重力の適用
	void ApplyGravity(MapChipField* mapChipField);
	// 範囲内のブロックを反転
	void InvertBlocksInArea(MapChipField* mapChipField);

private:
	using json = nlohmann::json;

	void SaveToJson();
	void LoadFromJson();
};