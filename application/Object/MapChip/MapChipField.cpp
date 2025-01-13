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

void MapChipField::InvertBlocksInArea(const Vector3& center)
{
	// 中心位置からマップ上のマス位置を計算
	int centerX = static_cast<int>(std::round(center.x / kChipSize));
	int centerY = static_cast<int>(std::round(-center.y / kChipSize));

	// プレイヤーの位置を中心に3x3マスを探索
	const int range = 1; // -1~1の範囲を探索するため

	// 3x3マス内のブロックを探索
	for (int y = -range; y <= range; ++y) {
		for (int x = -range; x <= range; ++x) {
			// 対象位置を計算
			int targetX = centerX + x;
			int targetY = centerY + y;

			// マップ範囲外を無視
			if(targetX < 0 || targetX >= static_cast<int>(kWidth) ||
				targetY < 0 || targetY >= static_cast<int>(kHeight)) {
				continue;
			}

			// ブロックを取得して反転処理
			MapChip& chip = mapChips_[targetY][targetX];
			// 黒ブロック->白ブロックに変更
			if (chip.type == ChipType::Black) {
				chip.type = ChipType::White;
				chip.object->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			// 白ブロック->黒ブロックに変更
			} else if (chip.type == ChipType::White) {
				chip.type = ChipType::Black;
				chip.object->SetObjColor({ 0.0f, 0.0f, 0.0f, 1.0f });
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
				chip.object->SetScale({ 0.925f, 0.925f, 0.925f }); // 一旦分かりやすいように少し小さくする
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
					chip.object->SetObjColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // 一旦分かりやすく緑に変更
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

void MapChipField::InvertBlocksWithCapture()
{
	// 8方向を表すオフセット（x, y）
	const std::vector<std::pair<int, int>> directions = {
		{0, -1}, {0, 1}, {-1, 0}, {1, 0}, // 上下左右
		{-1, -1}, {-1, 1}, {1, -1}, {1, 1} // 斜め方向
	};

	// ステージ全体のブロックを探索
	for (int y = 0; y < kHeight; ++y) {
		for (int x = 0; x < kWidth; ++x) {
			// 起点が黒ブロックの場合に白を挟み込む
			if (mapChips_[y][x].type == ChipType::Black) {
				ProcessCapture(x, y, ChipType::White, ChipType::Black, directions);
			}
			// 起点が白ブロックの場合に黒を挟み込む
			else if (mapChips_[y][x].type == ChipType::White) {
				ProcessCapture(x, y, ChipType::Black, ChipType::White, directions);
			}
		}
	}
}

void MapChipField::ProcessCapture(int startX, int startY, ChipType targetType, ChipType ownType, const std::vector<std::pair<int, int>>& directions)
{
	for (const auto& dir : directions) {
		std::vector<std::pair<int, int>> capturedBlocks;

		// 現在位置から方向に沿って探索
		int currX = startX + dir.first;
		int currY = startY + dir.second;

		while (IsValidPosition(currX, currY)) {
			// 対象タイプのブロックが見つかれば一時的にキャプチャ
			if (mapChips_[currY][currX].type == targetType) {
				capturedBlocks.emplace_back(currX, currY);
			}
			// 自分のタイプのブロックが見つかれば挟み込み成立
			else if (mapChips_[currY][currX].type == ownType) {
				for (const auto& block : capturedBlocks) {
					InvertBlock(block.first, block.second);
				}
				break;
			}
			// 空白や他のタイプのブロックの場合、挟み込み失敗
			else {
				break;
			}

			// 次の位置に進む
			currX += dir.first;
			currY += dir.second;
		}
	}
}

void MapChipField::InvertBlock(int x, int y)
{
	if (IsValidPosition(x, y)) {
		MapChip& chip = mapChips_[y][x];
		if (chip.type == ChipType::Black) {
			chip.type = ChipType::White;
			chip.object->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		} else if (chip.type == ChipType::White) {
			chip.type = ChipType::Black;
			chip.object->SetObjColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		}
	}
}

bool MapChipField::IsValidPosition(int x, int y) const
{
	return x >= 0 && x < static_cast<int>(kWidth) && y >= 0 && y < static_cast<int>(kHeight);
}