#pragma once

// Application
#include "application/Base/BaseObject.h"

class Block : public BaseObject 
{
public:
	// マップチップの種類を定義
	enum class ChipType {
		Empty,       // 空白ブロック
		Black,       // 黒ブロック
		White,       // 白ブロック
		Gray,        // 灰ブロック（動かないブロック）
		Goal,        // ゴール
		Player,      // プレイヤー初期位置
		Gravity,     // 重力反転ブロック
		ColorChange, // プレイヤー色反転ブロック
	};

	void Init(const std::string className) override;
	void Update() override;
	void Draw(const ViewProjection& viewProjection) override;

	ChipType type_;
};
