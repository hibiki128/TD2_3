#pragma once

// Engine
#include "Input.h"

// Application
#include "application/Base/BaseObject.h"
#include "application/Object/MapChip/MapChipField.h"

class Player : public BaseObject
{
public:
	void Init(const std::string className)override;
	void Update(MapChipField* mapChipField);
	void Draw(const ViewProjection& viewProjection)override;

private:
	// 入力
	Input* input_;

private:
	// 移動関数
	void Move();
	// 範囲内のブロックを反転
	void InvertBlocksInArea(MapChipField* mapChipField);
};

