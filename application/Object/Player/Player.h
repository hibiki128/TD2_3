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

private:
	// 入力
	Input* input_;

	///
	/// 基本的なパラメータ
	/// 
	Vector3 velocity_;

private:
	// 移動関数
	void Move();
	// 重力の適用
	void ApplyGravity();
	// 範囲内のブロックを反転
	void InvertBlocksInArea(MapChipField* mapChipField);

	void OnCollision([[maybe_unused]] Collider* other) override;
};