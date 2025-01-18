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
	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool hittingGround_ = false;
		bool hittingCeiling_ = false;
		bool hittingLeft_ = false;
		bool hittingRight_ = false;

		Block* blockX = nullptr; // X方向で衝突したブロック
		Block* blockY = nullptr; // Y方向で衝突したブロック
	};

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
	
	// マップとの当たり判定情報
	CollisionMapInfo collisionMapInfo_;
	// 微小な値
	const float kBlank = 0.0001f;

	// サイズ
	const float kWidth = 2.0f;
	const float kHeight = 2.0f;

	// 移動関連
	Vector3 velocity_; // 速度
	const float kMoveSpeed = 0.15f; // 移動速度

	// ジャンプ関連
	float gravityAcceleration_; // 重力加速度
	float jumpAcceleration; // ジャンプ初速

private:
	// 入力操作
	void HandleInput();
	// マップとの当たり判定情報を返す
	CollisionMapInfo GetMapCollisionInfo();

	// 衝突判定
	void OnCollision([[maybe_unused]] Collider* other)override;

	// マップチップフィールドを保持
	MapChipField* mapChipField_ = nullptr;
private:
	using json = nlohmann::json;

	void SaveToJson();
	void LoadFromJson();
};