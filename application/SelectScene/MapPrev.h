#pragma once

// C++
#include <stdint.h>
#include <vector>
#include <memory>

// Application
#include "application/Base/BaseObject.h"

class MapPrev
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

	MapPrev();

	void Init(const std::string& csvFilePath);
	void Update();
	void Draw(const ViewProjection& vp);
	void Debug();

private:
	// マップチップのデータ構造
	struct MapChip {
		std::unique_ptr<BaseObject> object;
		ChipType type;
	};

	// マップチップの二次元配列
	std::vector<std::vector<MapChip>> mapChips_;

	Vector3 center_;
	float rotationAngleY_;

	std::unique_ptr<BaseObject> centerObj_;
	
private:
	///
	///	CSVファイルからマップチップの読み込み
	/// 

	// CSVからマップチップデータを読み込む
	void LoadFromCSV(const std::string& filePath);
	// ChipTypeを整数値から取得する関数
	ChipType GetChipTypeFromInt(int value);

	void UpdateMapChipsPosition();
	
	Vector3 CalculateChipPosition(int x, int y);

	// 中心を基準にY軸回転を行う関数を追加
	Vector3 RotateAroundCenter(const Vector3& position, float angle);

	void RotationMap();

private:
	float timer_ = 0.0f;
};

