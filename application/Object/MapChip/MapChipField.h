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

	///
	/// 基本的な関数
	///

	MapChipField();

	void Init(const std::string& csvFilePath);
	void Update();
	void Draw(const ViewProjection& vp);

	///
	///	ブロックの反転処理（プレイヤー側で呼び出す）
	/// 

	// 3x3範囲内のブロックの色を反転（
	void InvertBlocksInArea(const Vector3& center);
	// 反転させて挟み込んだブロックの色の反転を行う
	void InvertBlocksWithCapture();

private:
	// マップチップのデータ構造
	struct MapChip {
		std::unique_ptr<BaseObject> object;
		ChipType type;
	};

	// マップチップの二次元配列
	std::vector<std::vector<MapChip>> mapChips_;

private:
	///
	///	CSVファイルからマップチップの読み込み
	/// 

	// CSVからマップチップデータを読み込む
	void LoadFromCSV(const std::string& filePath);
	// ChipTypeを整数値から取得する関数
	ChipType GetChipTypeFromInt(int value);

	///
	///	ブロックの挟み込み反転処理
	/// 
	
	// 挟み込み処理を汎用化
	void ProcessCapture(int startX, int startY, ChipType targetType, ChipType ownType, const std::vector<std::pair<int, int>>& directions);
	// 指定された座標のブロックを反転する
	void InvertBlock(int x, int y);
	// 指定された座標が有効範囲内か確認
	bool IsValidPosition(int x, int y) const;
};

