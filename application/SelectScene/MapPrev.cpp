#include "MapPrev.h"

// C++
#include <fstream>
#include <sstream>
#include <iostream>

// Engine
#include "math/Easing.h"

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
}


void MapPrev::Debug()
{
	ImGui::Begin("プレビューマップ");
	if (ImGui::DragFloat3("中心点", &center_.x, 0.1f)) {
		UpdateMapChipsPosition();
	}
	if (ImGui::DragFloat("回転角度", &rotationAngleY_)) {
		UpdateMapChipsPosition();
	}
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
	centerObj_->Draw(vp);
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