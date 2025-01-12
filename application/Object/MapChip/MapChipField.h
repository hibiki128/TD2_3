#pragma once

// C++
#include <stdint.h>
#include <vector>
#include <memory>

// Application
#include "application/Base/BaseObject.h"

class MapChipField
{
public:
	static const uint32_t kWidth = 13; // 横マス数
	static const uint32_t kHeight = 8; // 縦マス数
	static const float kChipSize; // 各マップチップのサイズ

	// マップチップの種類を定義
	enum class ChipType {
		Empty, // 空白ブロック
		Black, // 黒ブロック
		White, // 白ブロック
		Gray,  // 灰ブロック（動かないブロック）
	};

	MapChipField();

	void Init(const std::string& csvFilePath);
	void Update();
	void Draw(const ViewProjection& vp);

	// 範囲内のブロックを反転
	void InvertBlocksInArea(const Vector3& center);

private:
	// マップチップのデータ構造
	struct MapChip {
		std::unique_ptr<BaseObject> object;
		ChipType type;
	};

	// マップチップの二次元配列
	std::vector<std::vector<MapChip>> mapChips_;

private:
	// CSVからマップチップデータを読み込む
	void LoadFromCSV(const std::string& filePath);

	// ChipTypeを整数値から取得する関数
	ChipType GetChipTypeFromInt(int value);
};

