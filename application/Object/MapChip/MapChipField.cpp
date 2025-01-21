#include "MapChipField.h"

// C++
#include <fstream>
#include <sstream>
#include <iostream>

// Engine
#include "math/Easing.h"

// ブロックの大きさを定義
const float MapChipField::kChipSize = 2.0f;

MapChipField::MapChipField()
{
}

void MapChipField::Init(const std::string& csvFilePath)
{
	// CSVファイルからマップの読み込み
	LoadFromCSV(csvFilePath);
}

void MapChipField::Update()
{
	for (auto& row : mapChips_) {
		for (auto& chip : row) {
			// 空白ブロックではない場合のみ更新
			if (chip.object->type_ != Block::ChipType::Empty) {
				// アニメーションの進行
				if (chip.isAnimating) {
					UpdateChipAnimation(chip);
				// 通常の更新
				} else {
					chip.object->Update();
				}
			}
		}
	}

	// 挟み込みが起こった場合に挟まれたブロックの色反転を行う（たぶんここで呼んでるといつか問題起きるので呼び出し位置を検討）
	InvertBlocksWithCapture();


	// ゴールオブジェクト更新
	goal_->Update();
}

void MapChipField::Draw(const ViewProjection& vp)
{
	for (const auto& row : mapChips_) {
		for (const auto& chip : row) {
			// 空白ブロックではない場合のみ描画
			if (chip.object->type_ != Block::ChipType::Empty) {
				chip.object->Draw(vp);
			}
		}
	}


	// ゴールオブジェクト描画
	goal_->Draw(vp);
}

std::vector<Block*> MapChipField::GetBlocks() const { 
	std::vector<Block*> blocks; 

	for (const auto& row : mapChips_) {
		for (const auto& chip : row) {
			if (chip.object) {
				blocks.push_back(chip.object.get());
			}
		}
	}

	return blocks;
}

void MapChipField::InvertBlocksInArea(const Vector3& center, int xRange, int yRange)
{
	// 中心位置からマップ上のマス位置を計算
	int centerX = static_cast<int>(std::round(center.x / kChipSize));
	int centerY = static_cast<int>(std::round(-center.y / kChipSize));

	int halfXRange = xRange / 2;
	int halfYRange = yRange / 2;

	// 3x3マス内のブロックを探索
	for (int y = -halfYRange; y <= halfYRange; ++y) {
		for (int x = -halfXRange; x <= halfXRange; ++x) {
			// 対象位置を計算
			int targetX = centerX + x;
			int targetY = centerY + y;

			// マップ範囲外を無視
			if(targetX < 0 || targetX >= static_cast<int>(mapWidth) ||
				targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
				continue;
			}

			// ブロックを取得して反転処理（アニメーション開始）
			MapChip& chip = mapChips_[targetY][targetX];
			if (!chip.isAnimating && chip.object->type_ != Block::ChipType::Empty) {
				chip.isAnimating = true;
				chip.animState = MapChip::AnimationState::Shrinking;
				chip.animationTime = 0.0f;
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
	int maxHeight = 0; // 最大の行数を記録する変数

	// 最初にファイルの行数を数える
	while (std::getline(file, line)) {
		++maxHeight;
	}
	file.clear();
	file.seekg(0);

	// 二次元配列の要素数を設定
	mapChips_.resize(maxHeight);
	while (std::getline(file, line) && y < maxHeight) {
		std::istringstream lineStream(line);
		std::string cell;
		int x = 0;

		while (std::getline(lineStream, cell, ',')) {
			int chipValue = std::stoi(cell);
			Block::ChipType chipType = GetChipTypeFromInt(chipValue);

			MapChip chip;
			chip.object = std::make_unique<Block>();
			chip.object->type_ = chipType;

			/*BaseObjectの初期化*/

			// 空白ブロックの場合にはスキップ
			if (chip.object->type_ != Block::ChipType::Empty) {
				chip.object->Init("Block");
				chip.object->SetScale({1.0f, 1.0f, 1.0f});
				chip.object->SetWorldPosition({ x * kChipSize, -y * kChipSize, 0.0f });
				chip.object->CreateCollider();

				// モデルと色を設定
				switch (chip.object->type_) {
				case Block::ChipType::Black:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 0.0f, 0.0f, 0.0f, 1.0f }); // 黒色
					break;
				case Block::ChipType::White:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 白色
					break;
				case Block::ChipType::Gray:
					chip.object->CreateModel("debug/Cube.obj");
					chip.object->SetObjColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // 一旦分かりやすく緑に変更
					break;
				default:
					break;
				}
			}

			/*ゴールオブジェクトの生成*/
			if (chipValue == 4) {
				goal_ = std::make_unique<Goal>();
				goal_->Init("Goal");
				goal_->CreateModel("debug/ICO.obj");
				goal_->SetWorldPosition({ x * kChipSize, -y * kChipSize, 0.0f });
				goal_->CreateCollider();
				goal_->SetObjColor({ 1.0f, 1.0f, 0.0f, 1.0f }); // 黄色にしておく
			}

			/*プレイヤー初期位置の格納*/
			if (chipValue == 5) {
				playerInitialPosition_ = {x * kChipSize, -y * kChipSize, 0.0f};
			};

			// マップチップの二次元配列に格納
			mapChips_[y].push_back(std::move(chip));
			++x;
		}
		++y;
	}

	file.close();

	// mapWidthとmapHeightを再設定
	mapWidth = mapChips_[0].size();
	mapHeight = mapChips_.size();
}

Block::ChipType MapChipField::GetChipTypeFromInt(int value)
{
	switch (value) {
	case 0: return Block::ChipType::Empty;
	case 1: return Block::ChipType::Black;
	case 2: return Block::ChipType::White;
	case 3: return Block::ChipType::Gray;
	case 4: return Block::ChipType::Empty; // ゴールオブジェクトは空白扱いとする
	case 5: return Block::ChipType::Empty; // プレイヤー初期位置は空白扱いとする

	default: return Block::ChipType::Empty;
	}
}

void MapChipField::InvertBlocksWithCapture()
{
	// 8方向を表すオフセット（x, y）
	const std::vector<std::pair<int, int>> directions = {
		{0, -1}, {0, 1}, {-1, 0}, {1, 0}, // 上下左右
		//{-1, -1}, {-1, 1}, {1, -1}, {1, 1} // 斜め方向
	};

	// ステージ全体のブロックを探索
	for (int y = 0; y < mapHeight; ++y) {
		for (int x = 0; x < mapWidth; ++x) {
			// 起点が黒ブロックの場合に白を挟み込む
			if (mapChips_[y][x].object->type_ == Block::ChipType::Black) {
				ProcessCapture(x, y, Block::ChipType::White, Block::ChipType::Black, directions);
			}
			// 起点が白ブロックの場合に黒を挟み込む
			else if (mapChips_[y][x].object->type_ == Block::ChipType::White) {
				ProcessCapture(x, y, Block::ChipType::Black, Block::ChipType::White, directions);
			}
		}
	}
}

void MapChipField::ProcessCapture(int startX, int startY, Block::ChipType targetType, Block::ChipType ownType, const std::vector<std::pair<int, int>>& directions)
{
	for (const auto& dir : directions) {
		std::vector<std::pair<int, int>> capturedBlocks;

		// 現在位置から方向に沿って探索
		int currX = startX + dir.first;
		int currY = startY + dir.second;

		while (IsValidPosition(currX, currY)) {
			// 対象タイプのブロックが見つかれば一時的にキャプチャ
			if (mapChips_[currY][currX].object->type_ == targetType) {
				capturedBlocks.emplace_back(currX, currY);
			}
			// 自分のタイプのブロックが見つかれば挟み込み成立
			else if (mapChips_[currY][currX].object->type_ == ownType) {
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
	// 有効範囲内のブロックのみ
	if (IsValidPosition(x, y)) {
		MapChip& chip = mapChips_[y][x];

		// 動かないブロックの場合はスキップ
		if (chip.object->type_ == Block::ChipType::Gray) {
			return;
		}

		if (!chip.isAnimating && chip.object->type_ != Block::ChipType::Empty) {
			// アニメーション開始の遅延処理
			chip.isDelaying = true;
			chip.delayTime = 0.3f; // ここで指定した時間遅延

			chip.isAnimating = true; // 遅延後にアニメーションを開始するため有効化
		}
	}
}

bool MapChipField::IsValidPosition(int x, int y) const
{
	return x >= 0 && x < static_cast<int>(mapWidth) && y >= 0 && y < static_cast<int>(mapHeight);
}

void MapChipField::UpdateChipAnimation(MapChip& chip)
{
	// 動かないブロックの場合はスキップ
	if (chip.object->type_ == Block::ChipType::Gray) {
		chip.isAnimating = false; // 念のためアニメーション状態のリセット
		chip.animState = MapChip::AnimationState::None;
		return;
	}

	///
	///	アニメーション開始の遅延処理
	/// 

	if (chip.isDelaying) {
		chip.delayTime -= 1.0f / 60.0f; // フレーム減少

		if (chip.delayTime <= 0.0f) {
			chip.isDelaying = false; // 遅延終了
			chip.animState = MapChip::AnimationState::Shrinking;
			chip.animationTime = 0.0f;
		}
		return; // 遅延中はここで処理を終了する
	}

	constexpr float shrinkDuration = 0.2f; // 縮小時間
	constexpr float expandDuration = 0.2f; // 拡大時間

	///
	///	アニメーション処理開始
	/// 

	chip.animationTime += 1.0f / 60.0f; // フレーム進行

	/* ブロックの縮小->色変更->拡大の順番で処理を行う */

	///
	/// ブロックの収縮状態
	/// 
	if (chip.animState == MapChip::AnimationState::Shrinking) {
		// 終了した場合
		if (chip.animationTime >= shrinkDuration) {
			chip.animationTime = 0.0f; // タイマーリセット
			chip.animState = MapChip::AnimationState::ColorChange; // 次の状態に移行

			// ブロックの色変更
			if (chip.object->type_ == Block::ChipType::Black) {
				chip.object->type_ = Block::ChipType::White;
				chip.object->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			} else if (chip.object->type_ == Block::ChipType::White) {
				chip.object->type_ = Block::ChipType::Black;
				chip.object->SetObjColor({ 0.0f, 0.0f, 0.0f, 1.0f });
			}
		// 実際に収縮を行う
		} else {
			chip.currentScale = EaseOutQuad(1.0f, 0.5f, chip.animationTime, shrinkDuration); // スケールを { 1.0f -> 0.5f } へ縮小
			chip.object->SetScale({ chip.currentScale, chip.currentScale, chip.currentScale });
		}
	///
	/// ブロックの拡大状態
	/// 
	} else if (chip.animState == MapChip::AnimationState::Expanding) {
		// 終了した場合
		if (chip.animationTime >= expandDuration) {
			chip.animationTime = 0.0f; // タイマーリセット
			chip.animState = MapChip::AnimationState::None; // 次の状態に移行
			chip.isAnimating = false; // アニメーション終了
		// 実際に拡大を行う
		} else {
			chip.currentScale = EaseOutQuad(0.5f, 1.0f, chip.animationTime, expandDuration); // スケールを { 0.5f -> 1.0f } へ拡大
			chip.object->SetScale({ chip.currentScale, chip.currentScale, chip.currentScale });
		}
	///
	/// ブロックの色変更状態
	/// 
	} else if (chip.animState == MapChip::AnimationState::ColorChange) {
		// 次の状態に移行
		chip.animState = MapChip::AnimationState::Expanding;
		chip.animationTime = 0.0f;
	}
}
