#include "MapChipField.h"

// C++
#include <fstream>
#include <sstream>
#include <iostream>

// ブロックの大きさを定義
const float MapChipField::kChipSize = 2.0f;

MapChipField::MapChipField()
{
}

void MapChipField::Init(const std::string& csvFilePath)
{
	// 二次元配列の要素数を設定
	mapChips_.resize(kHeight);
	for (int y = 0; y < kHeight; ++y) {
		mapChips_[y].resize(kWidth);
	}

	// CSVファイルからマップの読み込み
	LoadFromCSV(csvFilePath);
}

void MapChipField::Update()
{
	for (auto& row : mapChips_) {
		for (auto& chip : row) {
			// 空白ブロックではない場合のみ更新
			if (chip.type != ChipType::Empty) {
				chip.object->Update();
			}
		}
	}
}

void MapChipField::Draw(const ViewProjection& vp)
{
	for (const auto& row : mapChips_) {
		for (const auto& chip : row) {
			// 空白ブロックではない場合のみ描画
			if (chip.type != ChipType::Empty) {
				chip.object->Draw(vp);
			}
		}
	}
}

void MapChipField::LoadFromCSV(const std::string& filePath)
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

			/*BaseObjectの初期化*/

			// 空白ブロックの場合にはスキップ
			if (chip.type != ChipType::Empty) {
				chip.object = std::make_unique<BaseObject>();
				chip.object->Init("MapChip");
				chip.object->SetWorldPosition({ x * kChipSize, -y * kChipSize, 0.0f });

				// モデルと色を設定
				switch (chip.type) {
				case ChipType::Black:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 0.0f, 0.0f, 0.0f, 1.0f }); // 黒色
					break;
				case ChipType::White:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 白色
					break;
				case ChipType::Gray:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 0.5f, 0.5f, 0.5f, 1.0f }); // 灰色
					break;
				default:
					break;
				}
			}

			// マップチップの二次元配列に格納
			mapChips_[y][x] = std::move(chip);
			++x;
		}
		++y;
	}

	file.close();
}

MapChipField::ChipType MapChipField::GetChipTypeFromInt(int value)
{
	switch (value) {
	case 0: return ChipType::Empty;
	case 1: return ChipType::Black;
	case 2: return ChipType::White;
	case 3: return ChipType::Gray;

	default: return ChipType::Empty;
	}
}
