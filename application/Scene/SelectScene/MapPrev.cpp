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
	csvFilePath_ = csvFilePath;

	// CSVファイルからマップの読み込み
	LoadFromCSV(csvFilePath_);
	rotationT_ = 0.0f;
	approachT_ = 0.0f;
	leaveT_ = 0.0f;
	dicisionT_ = 0.0f;
	center_.x = 0.5f;
	center_.y = -0.75f;
	finishT_ = 0.0f;
	isFinish_ = false;

	book_ = std::make_unique<BaseObject>();
	book_->Init("select_book");
	book_->CreateModel("select/book.obj");
	stageTex_ = std::make_unique<BaseObject>();
	stageTex_->Init("select_stageTex");
	stageTex_->CreateModel("clear/stage.obj");
	singleDigit_ = std::make_unique<BaseObject>();
	singleDigit_->Init("select_singleDigit");
	singleDigit_->CreateModel("clear/0.obj");
	twoDigit_ = std::make_unique<BaseObject>();
	twoDigit_->Init("select_twoDigit");
	twoDigit_->CreateModel("clear/0.obj");


	stageTex_->SetParent(book_->GetWorldTransform());
	singleDigit_->SetParent(book_->GetWorldTransform());
	twoDigit_->SetParent(book_->GetWorldTransform());
}

void MapPrev::Update()
{
	// マップチップの更新
	for (auto& row : mapChips_) {
		for (auto& chip : row) {
			if (chip.object->type_ != Block::ChipType::Empty) {
				chip.object->Update();
			}
		}
	}

	book_->Update();
	stageTex_->Update();
	singleDigit_->Update();
	twoDigit_->Update();

	MapMove();

	UpdateMapChipsPosition();

	FinishScene();

}


void MapPrev::Debug(const std::string& name)
{
	ImGui::Begin(name.c_str());
	ImGui::DragFloat3("中心点", &center_.x, 0.1f);
	ImGui::DragFloat("回転角度", &rotationAngleY_);
	ImGui::DragFloat("タイマー", &rotationT_, 0.1f);
	ImGui::Checkbox("選択中", &isSelect_);
	ImGui::Checkbox("決定", &isDecision_);
	ImGui::End();
	book_->DebugImGui();
	stageTex_->DebugImGui();
	singleDigit_->DebugImGui();
	twoDigit_->DebugImGui();
}


void MapPrev::Draw(const ViewProjection& vp)
{
	for (const auto& row : mapChips_) {
		for (const auto& chip : row) {
			// 空白ブロックではない場合のみ描画
			if (chip.object->type_ != Block::ChipType::Empty) {
				chip.object->Draw(vp);
			}
		}
	}
	book_->Draw(vp);
	stageTex_->Draw(vp);
	singleDigit_->Draw(vp);
	twoDigit_->Draw(vp);
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
				chip.object->SetScale({ 1.0f, 1.0f, 1.0f });
				chip.object->SetWorldPosition({ x * kChipSize, -y * kChipSize, 0.0f });
				chip.object->CreateCollider();

				// モデルと色を設定
				switch (chip.object->type_) {
				case Block::ChipType::Black: // 黒ブロック
					chip.object->CreateModel("game/blackBlock.obj");
					chip.object->SetTexture("game/blackBlock.png"); // 黒ブロックのテクスチャをセット
					break;
				case Block::ChipType::White: // 白ブロック
					chip.object->CreateModel("game/whiteBlock.obj");
					chip.object->SetTexture("game/whiteBlock.png"); // 白ブロックのテクスチャをセット
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

Block::ChipType MapPrev::GetChipTypeFromInt(int value)
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

void MapPrev::UpdateMapChipsPosition()
{
	for (int y = 0; y < mapHeight; ++y) {
		for (int x = 0; x < mapWidth; ++x) {
			auto& chip = mapChips_[y][x];
			if (chip.object->type_ != Block::ChipType::Empty) {
				// マップチップの位置を計算し、回転を適用
				Vector3 originalPos = CalculateChipPosition(x, y);
				Vector3 rotatedPos = RotateAroundCenter(originalPos, rotationAngleY_);

				// マップチップの位置を更新（回転は影響を与えない）
				chip.object->SetWorldPosition(rotatedPos);
			}
		}
	}
	book_->SetRotationY(degreesToRadians(-rotationAngleY_));
}

Vector3 MapPrev::CalculateChipPosition(int x, int y)
{
	// 中心からのオフセットを考慮してチップの位置を計算
	return {
		(x - mapWidth / 2.0f + center_.x) * kChipSize,
		-(y - mapHeight / 2.0f - center_.y) * kChipSize,
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
	if (!isDecision_) {
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
	const float targetAngle = 20.0f; // 目標角度
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
	const float endPos = 50.0f;
	const float startPosBook = book_->GetWorldPosition().z;
	const float endPosBook = 80.0f;
	const float easeTMax = 1.5f;

	// タイマーが上限を超えないように繰り返し増減させる
	approachT_ += Frame::DeltaTime();

	// timer_がeaseTMaxを超えたら方向を反転
	if (approachT_ >= easeTMax) {
		approachT_ = easeTMax;
	}

	center_.z = EaseInSine<float>(startPos, endPos, approachT_, easeTMax);
	
	book_->SetWorldPositionZ(EaseInSine<float>(startPosBook, endPosBook, approachT_, easeTMax));

}

void MapPrev::LeaveMap()
{
	const float startAngle = rotationAngleY_;
	const float endAngle = 0.0f;
	const float startPos = center_.z;
	const float endPos = 100.0f;
	const float startPosBook = book_->GetWorldPosition().z;
	const float endPosBook = 110.0f;
	const float easeTMax = 1.5f;

	// タイマーが上限を超えないように繰り返し増減させる
	leaveT_ += Frame::DeltaTime();

	// timer_がeaseTMaxを超えたら方向を反転
	if (leaveT_ >= easeTMax) {
		leaveT_ = easeTMax;
	}

	rotationAngleY_ = EaseInSine<float>(startAngle, endAngle, leaveT_, easeTMax);
	center_.z = EaseInSine<float>(startPos, endPos, leaveT_, easeTMax);
	
	book_->SetWorldPositionZ(EaseInSine<float>(startPosBook, endPosBook, leaveT_, easeTMax));

}

void MapPrev::DecisionMap()
{
	const float startPos = center_.z;
	const float endPos = 30.0f;
	const float startPosBook = book_->GetWorldPosition().z;
	const float endPosBook = 60.0f;
	const float easeTMax = 1.0f;
	const float startAngle = rotationAngleY_;
	const float endAngle = 0.0f;

	dicisionT_ += Frame::DeltaTime();
	if (dicisionT_ >= easeTMax) {
		dicisionT_ = easeTMax;
	}

	rotationAngleY_ = EaseInSine<float>(startAngle, endAngle, dicisionT_, easeTMax);
	center_.z = EaseOutQuint<float>(startPos, endPos, dicisionT_, easeTMax);
	
	book_->SetWorldPositionZ(EaseOutQuint<float>(startPosBook, endPosBook, dicisionT_, easeTMax));

}

void MapPrev::FinishScene()
{
	if (isDecision_) {
		finishT_ += Frame::DeltaTime();
		if (finishT_ > 1.0f) {
			isFinish_ = true;
		}
	}
}
