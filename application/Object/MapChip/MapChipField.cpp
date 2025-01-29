#include "MapChipField.h"

// C++
#include <fstream>
#include <sstream>
#include <iostream>

// Engine
#include "math/Easing.h"
#include"myEngine/Frame/Frame.h"
#include "myEngine/utility/graphics/TextureManager.h"
#include"Audio.h"

// ブロックの大きさを定義
const float MapChipField::kChipSize = 2.0f;

MapChipField::MapChipField()
{
}

void MapChipField::Init(const std::string& csvFilePath)
{
	// ファイルパスの保存をしておく
	csvFilePath_ = csvFilePath;

	// CSVファイルからマップの読み込み
	LoadFromCSV(csvFilePath_);

	invertSE_ = Audio::GetInstance()->LoadWave("action/inversion.wav");
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
				}
				else {
					chip.object->Update();
				}
			}
		}
	}

	// 挟み込みが起こった場合に挟まれたブロックの色反転を行う（たぶんここで呼んでるといつか問題起きるので呼び出し位置を検討）
	InvertBlocksWithCapture();

	GravityParticleUpdate();

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

void MapChipField::DrawParticle(const ViewProjection& vp)
{
	for (const auto& row : mapChips_) {
		for (const auto& chip : row) {
			// 空白ブロックではない場合のみ描画
			if (chip.object->type_ == Block::ChipType::Black || chip.object->type_ == Block::ChipType::White) {
				ParticleCommon::GetInstance()->SetBlendMode(BlendMode::kNormal);
				chip.normal_->Draw(vp);
			}
			if (chip.object->type_ == Block::ChipType::Gravity) {
				ParticleCommon::GetInstance()->SetBlendMode(BlendMode::kAdd);
				chip.arrow_->Draw(vp);
			}
		}
	}
}

void MapChipField::PlaySE()
{
	//for (auto& row : mapChips_) {
	//	for (auto& chip : row) {
	//		if (chip.isInvers) {
	//			Audio::GetInstance()->PlayWave(invertSE_, 0.1f);
	//			chip.isInvers = false;
	//		}
	//	}
	//}
}

void MapChipField::DebugImGui() {
	ImGui::Begin("MapChipField");

	ImGui::Checkbox("重力反転状態", &isGravityReversed_);

	ImGui::End();
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

void MapChipField::ResetMapChip() {
	// マップチップの二次元配列をクリアする
	mapChips_.clear();

	// Initを呼んでマップ再生成
	Init(csvFilePath_);

	// 全てのブロックの状態をリセット
	for (auto& row : mapChips_) {
		for (auto& chip : row) {
			chip.isAnimating = false;
			chip.isDelaying = false;
			chip.animState = MapChip::AnimationState::None;
			chip.animationTime = 0.0f;
			chip.delayTime = 0.0f;
			chip.currentScale = 1.0f;
		}
	}
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
			if (targetX < 0 || targetX >= static_cast<int>(mapWidth) ||
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
			chip.normal_ = std::make_unique<ParticleEmitter>();
			chip.arrow_ = std::make_unique<ParticleEmitter>();

			/*BaseObjectの初期化*/

			// 空白ブロックの場合にはスキップ
			if (chip.object->type_ != Block::ChipType::Empty) {
				chip.object->Init("Block");
				chip.object->SetScale({ 1.0f, 1.0f, 1.0f });
				chip.object->SetWorldPosition({ x * kChipSize, -y * kChipSize, 0.0f });
				chip.object->CreateCollider();
				chip.normal_->Initialize("reverse", "debug/sphere.obj");
				chip.arrow_->Initialize("arrow_up", "debug/plane.obj");
				chip.arrow_->SetTexture("Particle/Arrow.png");

				// モデルと色を設定
				switch (chip.object->type_) {
				case Block::ChipType::Black: // 黒ブロック
					chip.object->CreateModel("game/blackBlock.obj");
					chip.object->SetTexture("game/blackBlock.png"); // 黒ブロックのテクスチャをセット
					break;
				case Block::ChipType::White: // 白ブロック
					chip.object->CreateModel("game/noTouchWhiteBlock.obj");
					chip.object->SetTexture("game/noTouchWhiteBlock.png"); // 白ブロックのテクスチャをセット
					break;
				case Block::ChipType::Gray: // 動かないブロック
					chip.object->CreateModel("game/block.obj");
					chip.object->SetTexture("game/block.png"); // 動かないブロックのテクスチャをセット
					break;
				case Block::ChipType::Gravity: // 重力反転ブロック
					chip.object->CreateModel("game/gravityBlockDown.obj");
					chip.object->SetTexture("game/gravityBlockDown.png"); // 重力通常状態のテクスチャをセット
					break;
				case Block::ChipType::ColorChange: // プレイヤー色変更ブロック
					chip.object->CreateModel("game/ColorChangeBlock.obj");
					chip.object->SetTexture("game/colorChangeBlock.png"); // プレイヤー色変更ブロックのテクスチャをセット
					break;
				default:
					break;
				}
			}

			/*ゴールオブジェクトの生成*/
			if (chipValue == 4) {
				goal_ = std::make_unique<Goal>();
				goal_->Init("Goal");
				goal_->CreateModel("game/goal.obj");
				goal_->SetTexture("game/goal.png");
				goal_->SetWorldPosition({ x * kChipSize, -y * kChipSize, 0.0f });
				goal_->CreateCollider();
				goal_->SetObjColor({ 1.0f, 1.0f, 0.0f, 1.0f }); // 黄色にしておく
			}

			/*プレイヤー初期位置の格納*/
			if (chipValue == 5) {
				playerInitialPosition_ = { x * kChipSize, -y * kChipSize, 0.0f };
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
	case 6: return Block::ChipType::Gravity;
	case 7: return Block::ChipType::ColorChange;

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

void MapChipField::GravityParticleUpdate()
{
	if (isGravityReversed_ != prevGravityState) {
		arrowTime_ = 1.0f;
	}

	if (arrowTime_ > 0.0f) {
		arrowTime_ -= Frame::DeltaTime();
		for (const auto& row : mapChips_) {
			for (const auto& chip : row) {
				if (chip.object->type_ == Block::ChipType::Gravity) {
					if (isGravityReversed_ && !prevGravityState) {
						chip.arrow_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
						chip.arrow_->LoadFromJson("arrow_up");
					}
					if (!isGravityReversed_ && prevGravityState) {
						chip.arrow_->SetColor({ 0.0f,0.0f,1.0f,1.0f });
						chip.arrow_->LoadFromJson("arrow_down");
					}
					chip.arrow_->SetPosition({ chip.object->GetCenterPosition().x,chip.object->GetCenterPosition().y,-1.5f });
					// 重力ブロックのみ
					chip.arrow_->Update();
				}
			}
		}
	}

	// 現在の状態を前回の状態として保存
	prevGravityState = isGravityReversed_;
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
	/// アニメーション開始の遅延処理
	///

	if (chip.isDelaying) {
		chip.delayTime -= 1.0f / 60.0f; // フレーム減少

		if (chip.delayTime <= 0.0f) {
			chip.isDelaying = false; // 遅延終了
			chip.animState = MapChip::AnimationState::Shrinking; // 回転アニメーションに移行
			chip.animationTime = 0.0f;
		}
		return; // 遅延中はここで処理を終了する
	}

	constexpr float rotationDuration = 0.4f; // 回転アニメーションの時間
	constexpr float halfRotationTime = rotationDuration / 2.0f; // 半回転のタイミング

	///
	/// アニメーション処理開始
	///

	chip.animationTime += 1.0f / 60.0f; // フレーム進行

	/* ブロックの一回転 -> 拡縮 -> 半回転時に色変更 -> 終了 の順番で処理を行う */

	///
	/// ブロックの回転およびスケールアニメーション
	///
	if (chip.animState == MapChip::AnimationState::Shrinking) {
		// 終了した場合
		if (chip.animationTime >= rotationDuration) {
			chip.animationTime = 0.0f; // タイマーリセット
			chip.animState = MapChip::AnimationState::None; // 次の状態に移行
			chip.isAnimating = false; // アニメーション終了
			chip.hasColorChanged = false;

			// 最終的に回転角度とスケールをリセットして終了
			chip.currentRotation = 0.0f;
			chip.currentScale = 1.0f;
			chip.object->SetRotation({ 0.0f, chip.currentRotation, 0.0f });
			chip.object->SetScale({ chip.currentScale, chip.currentScale, chip.currentScale });

			chip.normal_->SetPosition(chip.object->GetCenterPosition());

			if (chip.object->type_ == Block::ChipType::Black) {
				chip.normal_->SetTexture("particle/blackBlock1x1.png");
			}
			if (chip.object->type_ == Block::ChipType::White) {
				chip.normal_->SetTexture("particle/whiteBlock1x1.png");
			}

			if (chip.object->type_ == Block::ChipType::Black || chip.object->type_ == Block::ChipType::White) {
				chip.normal_->UpdateOnce();
			}

		}
		else {
			// 回転角度を更新
			chip.currentRotation = EaseOutQuad(0.0f, 720.0f, chip.animationTime, rotationDuration); // 回転を0° -> 360°へ
			chip.object->SetRotation({ 0.0f, degreesToRadians(chip.currentRotation), 0.0f });

			// スケールの更新
			if (chip.animationTime <= halfRotationTime) {
				// 半回転までは縮小
				chip.currentScale = EaseOutQuad(1.0f, 0.5f, chip.animationTime, halfRotationTime); // スケールを1.0 -> 0.5へ縮小
			}
			else {
				// 半回転以降は拡大
				float timeSinceHalf = chip.animationTime - halfRotationTime;
				chip.currentScale = EaseOutQuad(0.5f, 1.0f, timeSinceHalf, halfRotationTime); // スケールを0.5 -> 1.0へ拡大
			}
			chip.object->SetScale({ chip.currentScale, chip.currentScale, chip.currentScale });
			// 半回転のタイミングで色を変更
			if (chip.animationTime >= halfRotationTime && !chip.hasColorChanged) {
				chip.hasColorChanged = true; // 色変更が一度だけ行われるようにフラグを設定
				// ブロックの色変更
				if (chip.object->type_ == Block::ChipType::Black) {
					chip.object->type_ = Block::ChipType::White;
					chip.object->CreateModel("game/noTouchWhiteBlock.obj");
					chip.object->SetTexture("game/noTouchWhiteBlock.png"); // 白ブロックのテクスチャをセット
				}
				else if (chip.object->type_ == Block::ChipType::White) {
					chip.object->type_ = Block::ChipType::Black;
					chip.object->CreateModel("game/blackBlock.obj");
					chip.object->SetTexture("game/blackBlock.png"); // 黒ブロックのテクスチャをセット
				}
			}
		}
	}
}

void MapChipField::ChangeTextureAllGravityBlock() {

	// 全てのブロックを探索
	for (auto& row : mapChips_) {
		for (auto& chip : row) {
			if (chip.object->type_ == Block::ChipType::Gravity) { // 重力ブロックの場合
				if (!isGravityReversed_) {
					chip.object->CreateModel("game/gravityBlockDown.obj"); // 重力通常状態のテクスチャを設定
					chip.object->SetTexture("game/gravityBlockDown.png"); // 重力通常状態のテクスチャを設定
				}
				else {
					chip.object->CreateModel("game/gravityBlockUp.obj"); // 重力反転状態のテクスチャを設定
					chip.object->SetTexture("game/gravityBlockUp.png"); // 重力反転状態のテクスチャを設定
				}
			}
		}
	}
}

bool MapChipField::HasBlockInArea(const Vector3& center, int xRange, int yRange) {
	// 中心位置からマップ上のマス位置を計算
	int centerX = static_cast<int>(std::round(center.x / kChipSize));
	int centerY = static_cast<int>(std::round(-center.y / kChipSize));

	int halfXRange = xRange / 2;
	int halfYRange = yRange / 2;

	// 範囲内のブロックを探索
	for (int y = -halfYRange; y <= halfYRange; ++y) {
		for (int x = -halfXRange; x <= halfXRange; ++x) {
			// 対象位置を計算
			int targetX = centerX + x;
			int targetY = centerY + y;

			// マップ範囲外を無視
			if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
				continue;
			}

			// マスにブロックが存在するかをチェック
			MapChip& chip = mapChips_[targetY][targetX];
			if (chip.object->type_ != Block::ChipType::Empty && // 空ブロックを除外する
				chip.object->type_ != Block::ChipType::Gray) { // 動かないブロックを除外する（あとで変更する可能性あり）
				return true;
			}
		}
	}

	return false;
}

bool MapChipField::HasGravityBlockInArea(const Vector3& center, int xRange, int yRange) {
	// 中心位置からマップ上のマス位置を計算
	int centerX = static_cast<int>(std::round(center.x / kChipSize));
	int centerY = static_cast<int>(std::round(-center.y / kChipSize));

	int halfXRange = xRange / 2;
	int halfYRange = yRange / 2;

	// 範囲内のブロックを探索
	for (int y = -halfYRange; y <= halfYRange; ++y) {
		for (int x = -halfXRange; x <= halfXRange; ++x) {
			// 対象位置を計算
			int targetX = centerX + x;
			int targetY = centerY + y;

			// マップ範囲外を無視
			if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
				continue;
			}

			// ブロックを取得してタイプを判定
			MapChip& chip = mapChips_[targetY][targetX];
			if (chip.object->type_ == Block::ChipType::Gravity) { // 重力ブロックが見つかったら
				isGravityReversed_ = !isGravityReversed_; // 重力の状態を切り替える

				// 重力の状態によって全ての重力ブロックのテクスチャを切り替える
				ChangeTextureAllGravityBlock();

				return true; // 重力ブロックが見つかったことを示す
			}
		}
	}

	return false;
}

bool MapChipField::HasColorChangeBlockInArea(const Vector3& center, int xRange, int yRange)
{
	// 中心位置からマップ上のマス位置を計算
	int centerX = static_cast<int>(std::round(center.x / kChipSize));
	int centerY = static_cast<int>(std::round(-center.y / kChipSize));

	int halfXRange = xRange / 2;
	int halfYRange = yRange / 2;

	// 範囲内のブロックを探索
	for (int y = -halfYRange; y <= halfYRange; ++y) {
		for (int x = -halfXRange; x <= halfXRange; ++x) {
			// 対象位置を計算
			int targetX = centerX + x;
			int targetY = centerY + y;

			// マップ範囲外を無視
			if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
				continue;
			}

			// ブロックを取得してタイプを判定
			MapChip& chip = mapChips_[targetY][targetX];
			if (chip.object->type_ == Block::ChipType::ColorChange) { // プレイヤー色反転ブロックが見つかったら

				return true; // プレイヤー色反転ブロックが見つかったことを示す
			}
		}
	}

	return false;
}
