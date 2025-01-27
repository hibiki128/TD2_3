#pragma once

// C++
#include <stdint.h>
#include <vector>
#include <memory>

// Application
#include "application/Base/BaseObject.h"
#include "application/Object/MapChip/Block/Block.h"
#include "application/Object/Goal/Goal.h"

class MapPrev
{
public:
	size_t mapWidth = 13; // 横マス数
	size_t mapHeight = 8; // 縦マス数
	static const float kChipSize; // 各マップチップのサイズ

	///
	/// 基本的な関数
	///

	MapPrev();

	void Init(const std::string& csvFilePath);
	void Update();
	void Draw(const ViewProjection& vp);
	void Debug(std::string& name);

	bool GetIsSelect() { return isSelect_; }
	bool GetDecision() { return isDecision_; }
	void SetIsSelect(bool isSelect) { isSelect_ = isSelect; }
	void SetDecision(bool isDecision) { isDecision_ = isDecision; }
	void SetPosition(const Vector3 position) { center_ = position; }
	bool IsFinish() { return isFinish_; }

private:
	// マップチップのデータ構造
	struct MapChip {
		std::unique_ptr<Block> object;
	};

	// マップチップの二次元配列
	std::vector<std::vector<MapChip>> mapChips_;
	std::string csvFilePath_; // ファイルパス保存用

	Vector3 center_;
	float rotationAngleY_;
	
private:
	///
	///	CSVファイルからマップチップの読み込み
	/// 

	// CSVからマップチップデータを読み込む
	void LoadFromCSV(const std::string& filePath);
	// ChipTypeを整数値から取得する関数
	Block::ChipType GetChipTypeFromInt(int value);
	
	void UpdateMapChipsPosition();
	
	Vector3 CalculateChipPosition(int x, int y);

	// 中心を基準にY軸回転を行う関数を追加
	Vector3 RotateAroundCenter(const Vector3& position, float angle);

	void MapMove();

	void RotationMap();

	void ApproachMap();

	void LeaveMap();

	void DecisionMap();

	void FinishScene();
private:

	// -----各イージング用 T-----
	float rotationT_ = 0.0f;
	float approachT_ = 0.0f;
	float leaveT_ = 0.0f;
	float dicisionT_ = 0.0f;

	// -----イージング用フラグ-----
	bool isSelect_ = false;
	bool isDecision_ = false;
	float startAngle_;

	bool isFinish_ = false;
	float finishT_ = 0.0f;
};

