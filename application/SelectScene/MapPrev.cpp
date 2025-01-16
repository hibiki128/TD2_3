#include "MapPrev.h"

// C++
#include <fstream>
#include <sstream>
#include <iostream>

// Engine
#include "math/Easing.h"
#include <myEngine/Frame/Frame.h>

// ブロックの大きさを定義
const float MapPrev::kChipSize = 2.0f;

MapPrev::MapPrev()
	: center_({
		0.0f,
		0.0f,
		60.0f
		})
{
}

void MapPrev::Init(const std::string& csvFilePath)
{
	// 二次元配列の要素数を設定
	mapChips_.resize(kHeight);
	for (int y = 0; y < kHeight; ++y) {
		mapChips_[y].resize(kWidth);
	}
	
	centerObj_ = std::make_unique<BaseObject>();
	centerObj_->Init("center");
	centerObj_->CreateModel("debug/sphere.obj");

	// CSVファイルからマップの読み込み
	LoadFromCSV(csvFilePath);
	rotationT_ = 0.0f;
	approachT_ = 0.0f;
	leaveT_ = 0.0f;
	dicisionT_ = 0.0f;
	center_.x = 0.5f;
	center_.y = -0.75f;
}

void MapPrev::Update()
{
	// マップチップの更新
	for (auto& row : mapChips_) {
		for (auto& chip : row) {
			if (chip.type != ChipType::Empty) {
				chip.object->Update();
			}
		}
	}

	// 中心オブジェクトの更新
	centerObj_->SetWorldPosition({ center_.x * kChipSize, center_.y * kChipSize, center_.z });
	centerObj_->Update();

	MapMove();

	UpdateMapChipsPosition();
}


void MapPrev::Debug(std::string& name)
{
	ImGui::Begin(name.c_str());
	ImGui::DragFloat3("中心点", &center_.x, 0.1f);
	ImGui::DragFloat("回転角度", &rotationAngleY_);
	ImGui::DragFloat("タイマー", &rotationT_, 0.1f);
	ImGui::Checkbox("選択中", &isSelect_);
	ImGui::Checkbox("決定", &isDecision);
	ImGui::End();
}


void MapPrev::Draw(const ViewProjection& vp)
{
	for (const auto& row : mapChips_) {
		for (const auto& chip : row) {
			// 空白ブロックではない場合のみ描画
			if (chip.type != ChipType::Empty) {
				chip.object->Draw(vp);
			}
		}
	}
	//centerObj_->Draw(vp);
}

void MapPrev::LoadFromCSV(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open()) {
		return;
	}

	std::string line;
	int y = 0;

	while (std::getline(file, line) && y < kHeight) {
		std::istringstream lineStream(line);
		std::string cell;
		int x = 0;

		while (std::getline(lineStream, cell, ',') && x < kWidth) {
			int chipValue = std::stoi(cell);
			ChipType chipType = GetChipTypeFromInt(chipValue);

			MapChip chip;
			chip.type = chipType;

			if (chip.type != ChipType::Empty) {
				chip.object = std::make_unique<BaseObject>();
				chip.object->Init("MapChip");
				chip.object->SetScale({ 0.925f, 0.925f, 0.925f });
				chip.object->SetWorldPosition({
					x * kChipSize,
					-y * kChipSize,
					0.0f // z方向の位置を追加
					});

				// モデルと色を設定
				switch (chip.type) {
				case ChipType::Black:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 0.0f, 0.0f, 0.0f, 1.0f });
					break;
				case ChipType::White:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f });
					break;
				case ChipType::Gray:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 0.0f, 1.0f, 0.0f, 1.0f });
					break;
				default:
					break;
				}
			}

			mapChips_[y][x] = std::move(chip);
			++x;
		}
		++y;
	}

	file.close();
	UpdateMapChipsPosition();
}

MapPrev::ChipType MapPrev::GetChipTypeFromInt(int value)
{
	switch (value) {
	case 0: return ChipType::Empty;
	case 1: return ChipType::Black;
	case 2: return ChipType::White;
	case 3: return ChipType::Gray;

	default: return ChipType::Empty;
	}
}

void MapPrev::UpdateMapChipsPosition()
{
	for (int y = 0; y < kHeight; ++y) {
		for (int x = 0; x < kWidth; ++x) {
			auto& chip = mapChips_[y][x];
			if (chip.type != ChipType::Empty) {
				Vector3 originalPos = CalculateChipPosition(x, y);
				Vector3 rotatedPos = RotateAroundCenter(originalPos, rotationAngleY_);

				// 位置のみを更新し、オブジェクトの回転には影響を与えない
				chip.object->SetWorldPosition(rotatedPos);

			}
		}
	}
}



Vector3 MapPrev::CalculateChipPosition(int x, int y)
{
	// 中心からのオフセットを考慮してチップの位置を計算
	return {
		(x - kWidth / 2.0f + center_.x) * kChipSize,
		-(y - kHeight / 2.0f - center_.y) * kChipSize,
		center_.z
	};

}
// 中心を基準にY軸回転を行う関数を追加
Vector3 MapPrev::RotateAroundCenter(const Vector3& position, float angle)
{
	float radians = angle * (3.14159265f / 180.0f); // 角度をラジアンに変換
	float cosTheta = std::cos(radians);
	float sinTheta = std::sin(radians);

	float xOffset = position.x - center_.x * kChipSize;
	float zOffset = position.z - center_.z;

	float rotatedX = xOffset * cosTheta - zOffset * sinTheta + center_.x * kChipSize;
	float rotatedZ = xOffset * sinTheta + zOffset * cosTheta + center_.z;

	return { rotatedX, position.y, rotatedZ };
}

void MapPrev::MapMove()
{
	if (!isDecision) {
		dicisionT_ = 0.0f;
		if (isSelect_) {
			leaveT_ = 0.0f;
			ApproachMap();
			if (approachT_ > 1.0f) {
				RotationMap();
			}
		}
		else {
			approachT_ = 0.0f;
			rotationT_ = 0.0f;
			startAngle_ = rotationAngleY_;
			LeaveMap();
		}
	}
	else {
		DecisionMap();
	}
}

void MapPrev::RotationMap()
{
	const float targetAngle = 15.0f; // 目標角度
	const float easeTMax = 1.5f;

	// 初回の呼び出し時に前回の角度を開始角度として使用

	static bool isReversing = false;

	// タイマーを更新
	rotationT_ += Frame::DeltaTime();

	// タイマーが上限を超えたら方向を反転し、角度を切り替え
	if (rotationT_ >= easeTMax) {
		rotationT_ = 0.0f;  // タイマーをリセット
		isReversing = !isReversing;  // 方向を反転

		// 新しい開始角度を設定
		startAngle_ = rotationAngleY_;
	}

	// 終了角度を設定
	float endAngle = isReversing ? -targetAngle : targetAngle;

	// イージング関数を使用して回転角度を計算
	rotationAngleY_ = EaseInOutQuad<float>(startAngle_, endAngle, rotationT_, easeTMax);
}

void MapPrev::ApproachMap()
{
	const float startPos = center_.z;
	const float endPos = 30.0f;
	const float easeTMax = 1.5f;

	// タイマーが上限を超えないように繰り返し増減させる
	approachT_ += Frame::DeltaTime();

	// timer_がeaseTMaxを超えたら方向を反転
	if (approachT_ >= easeTMax) {
		approachT_ = easeTMax;
	}

	center_.z = EaseInSine<float>(startPos, endPos, approachT_, easeTMax);

}

void MapPrev::LeaveMap()
{
	const float startAngle = rotationAngleY_;
	const float endAngle = 0.0f;
	const float startPos = center_.z;
	const float endPos = 60.0f;
	const float easeTMax = 1.5f;

	// タイマーが上限を超えないように繰り返し増減させる
	leaveT_ += Frame::DeltaTime();

	// timer_がeaseTMaxを超えたら方向を反転
	if (leaveT_ >= easeTMax) {
		leaveT_ = easeTMax;
	}

	rotationAngleY_ = EaseInSine<float>(startAngle, endAngle, leaveT_, easeTMax);
	center_.z = EaseInSine<float>(startPos, endPos, leaveT_, easeTMax);

}

void MapPrev::DecisionMap()
{
	const float startPos = center_.z;
	const float endPos = 0.0f;
	const float easeTMax = 0.5f;
	const float startAngle = rotationAngleY_;
	const float endAngle = 0.0f;

	dicisionT_ += Frame::DeltaTime();
	if (dicisionT_ >= easeTMax) {
		dicisionT_ = easeTMax;
	}

	rotationAngleY_ = EaseInSine<float>(startAngle, endAngle, dicisionT_, easeTMax);
	center_.z = EaseOutQuint<float>(startPos, endPos, dicisionT_, easeTMax);

}
