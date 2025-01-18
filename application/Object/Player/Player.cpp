#define NOMINMAX
#include "Player.h"

void Player::Init(const std::string className) {
	input_ = Input::GetInstance();

	BaseObject::Init(className);
	BaseObject::CreateModel("debug/Cube.obj");
	BaseObject::CreateCollider();
	BaseObject::SetObjColor({1.0f, 0.0f, 0.0f, 1.0f});

	// 初期位置の設定（一旦雑にここで）
	const int x = 6;
	const int y = 4;
	BaseObject::SetWorldPosition({x * MapChipField::kChipSize, y * -MapChipField::kChipSize, 0.0f});

	///
	///	各パラメーター初期化
	///		
	
	gravityAcceleration_ = -0.01f; // 重力
	jumpAcceleration = 0.3f; // ジャンプ初速

	// Jsonからパラメーターの読み込み
	LoadFromJson();
}

void Player::Update(MapChipField* mapChipField) {
	BaseObject::Update();

	///
	///	毎フレーム初期化処理
	/// 

	mapChipField_ = mapChipField;

	// 接地しているか天井に接触した際にはY方向速度をリセット
	if (collisionMapInfo_.hittingGround_) {
		velocity_.y = 0.0f;
	} else if (collisionMapInfo_.hittingCeiling_) {
		velocity_.y = 0.0f;
	}

	///
	///	入力操作
	/// 

	HandleInput();

	///
	///	重力を常に受ける
	///		

	velocity_.y += gravityAcceleration_;

	///
	///	全てのブロックとの衝突判定とプレイヤーの押し戻し
	/// 
	
	CheckCollisionAndResolve();

#ifdef _DEBUG
	ImGui::Begin("player");

	ImGui::DragFloat3("velocity", &velocity_.x);

	ImGui::Text("hittingGround : %d", collisionMapInfo_.hittingGround_);
	ImGui::Text("hittingCeiling : %d", collisionMapInfo_.hittingCeiling_);
	ImGui::Text("hittingLeft : %d", collisionMapInfo_.hittingLeft_);
	ImGui::Text("hittingRight : %d", collisionMapInfo_.hittingRight_);

	ImGui::End();
#endif
}

void Player::Draw(const ViewProjection& viewProjection) { 
	BaseObject::Draw(viewProjection); 
}

void Player::DebugImGui() {
	// デフォルトデバッグ表示（トランスフォーム、コライダー）
	BaseObject::DebugImGui(); 

	// 追加分デバッグ表示
	ImGui::Begin("player");
	if (ImGui::BeginTabBar(className_.c_str())) {
		if (ImGui::BeginTabItem("パラメーター調整")) {

			// なんか追加する場合こっから
			ImGui::DragFloat("重力加速度", &gravityAcceleration_, 0.001f);
			ImGui::DragFloat("ジャンプ初速", &jumpAcceleration, 0.01f);

			if (ImGui::Button("セーブ")) {
				SaveToJson();
				std::string message = std::format("Parameters saved.");
				MessageBoxA(nullptr, message.c_str(), "Object", 0);
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

bool Player::IsGoalReached()
{
	// 現在位置の取得
	Vector3 position = this->transform_.translation_;
	// プレイヤーの4つの角を計算
	Vector3 corners[4] = {
		{position.x - kWidth / 2, position.y + kHeight / 2, position.z}, // 左上
		{position.x + kWidth / 2, position.y + kHeight / 2, position.z}, // 右上
		{position.x - kWidth / 2, position.y - kHeight / 2, position.z}, // 左下
		{position.x + kWidth / 2, position.y - kHeight / 2, position.z}  // 右下
	};

	// ゴール位置の取得
	Vector3 goalPosition = mapChipField_->GetGoal()->GetWorldPosition();
	float goalLeft = goalPosition.x - MapChipField::kChipSize / 2;
	float goalRight = goalPosition.x + MapChipField::kChipSize / 2;
	float goalTop = goalPosition.y + MapChipField::kChipSize / 2;
	float goalBottom = goalPosition.y - MapChipField::kChipSize / 2;

	// 各角がゴール内にあるかを判定
	for (const auto& corner : corners) {
		if (corner.x >= goalLeft && corner.x <= goalRight &&
			corner.y >= goalBottom && corner.y <= goalTop) {
			return true; // 4つ角のどれかが触れていたらtrue
		}
	}

	return false;
}

void Player::HandleInput()
{
#pragma region ゲームパッド入力

#pragma endregion


# pragma region キーボード入力
	///
	///	左右移動入力
	/// 

	if (input_->PushKey(DIK_A)) {
		velocity_.x = -kMoveSpeed;}
	if (input_->PushKey(DIK_D)) {
		velocity_.x = kMoveSpeed;
	}

	///
	///	ジャンプ入力
	/// 
	
	if (input_->TriggerKey(DIK_W)) {
		// 地面にいる場合のみ
		if (collisionMapInfo_.hittingGround_) {
			velocity_.y = jumpAcceleration; // 上昇開始
		}
	}

	///
	///	範囲内のブロック反転入力
	/// 
	
	if (input_->TriggerKey(DIK_SPACE)) {
		if (mapChipField_) {
			// 現在の位置を取得
			Vector3 position = BaseObject::GetWorldPosition();
			// 範囲内のブロックの反転を行う
			mapChipField_->InvertBlocksInArea(position);
		}
	}
#pragma endregion
}

void Player::CheckCollisionAndResolve()
{
	/// X移動
	BaseObject::transform_.translation_.x += velocity_.x;

	/// 衝突判定
	CollisionMapInfo collisionMapInfoX = GetMapCollisionInfo();

	/// 押し戻し
	if (collisionMapInfoX.hittingLeft_) {
		Vector3 blockPosition = collisionMapInfoX.blockX->GetWorldPosition();
		float blockRight = blockPosition.x + MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.x = blockRight + kWidth / 2 + kBlank; // 左側に衝突した場合、右に押し戻し
	} else if (collisionMapInfoX.hittingRight_) {
		Vector3 blockPosition = collisionMapInfoX.blockX->GetWorldPosition();
		float blockLeft = blockPosition.x - MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.x = blockLeft - kWidth / 2 - kBlank; // 右側に衝突した場合、左に押し戻し
	}

	/// Y移動
	BaseObject::transform_.translation_.y += velocity_.y;

	/// 衝突判定
	CollisionMapInfo collisionMapInfoY = GetMapCollisionInfo();

	/// 押し戻し
	if (collisionMapInfoY.hittingGround_) {
		Vector3 blockPosition = collisionMapInfoY.blockY->GetWorldPosition();
		float blockBottom = blockPosition.y + MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.y = blockBottom + kHeight / 2 + kBlank; // 地面の位置に押し戻し
	} else if (collisionMapInfoY.hittingCeiling_) {
		Vector3 blockPosition = collisionMapInfoY.blockY->GetWorldPosition();
		float blockTop = blockPosition.y - MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.y = blockTop - kHeight / 2 - kBlank; // 天井の位置に押し戻し
	}

	/// 衝突判定を格納
	collisionMapInfo_.hittingGround_ = collisionMapInfoY.hittingGround_;
	collisionMapInfo_.hittingCeiling_ = collisionMapInfoY.hittingCeiling_;

	collisionMapInfo_.hittingLeft_ = collisionMapInfoX.hittingLeft_;
	collisionMapInfo_.hittingRight_ = collisionMapInfoX.hittingRight_;

	/// 速度リセット
	velocity_.x = 0.0f;
	/*velocity_.y = 0.0f;*/
}

 Player::CollisionMapInfo Player::GetMapCollisionInfo()
{
	 CollisionMapInfo info;

	// 現在位置の取得
	Vector3 position = this->transform_.translation_;
	// プレイヤーの4つの角を計算
	Vector3 corners[4] = {
		{position.x - kWidth / 2, position.y + kHeight / 2, position.z}, // 左上
		{position.x + kWidth / 2, position.y + kHeight / 2, position.z}, // 右上
		{position.x - kWidth / 2, position.y - kHeight / 2, position.z}, // 左下
		{position.x + kWidth / 2, position.y - kHeight / 2, position.z}  // 右下
	};

	// 全てのブロックを取得
	const auto blocks = mapChipField_->GetBlocks();
	const float blockSize = MapChipField::kChipSize;
	
	// 全てのブロックとの衝突判定
	for (const auto& block : blocks) {
		// ブロックの位置と範囲を計算
		Vector3 blockPosition = block->GetWorldPosition();
		float blockLeft = blockPosition.x - blockSize / 2;
		float blockRight = blockPosition.x + blockSize / 2;
		float blockTop = blockPosition.y + blockSize / 2;
		float blockBottom = blockPosition.y - blockSize / 2;

		// 各角の衝突を判定
		for (int i = 0; i < 4; ++i) {
			if (corners[i].x >= blockLeft && corners[i].x <= blockRight &&
				corners[i].y >= blockBottom && corners[i].y <= blockTop) {
				// 上下判定
				if (i < 2) {
					info.hittingCeiling_ = true;  // 左上・右上
					info.blockY = block; // Y方向で衝突したブロックを格納
				}
				if (i >= 2) {
					info.hittingGround_ = true; // 左下・右下
					info.blockY = block; // Y方向で衝突したブロックを格納
				}
				// 左右判定
				if (corners[i].x < blockPosition.x) {
					info.hittingRight_ = true;
					info.blockX = block; // X方向で衝突したブロックを格納
				}
				if (corners[i].x > blockPosition.x) {
					info.hittingLeft_ = true;
					info.blockX = block; // X方向で衝突したブロックを格納
				}
			}
		}
	}

	return info;
}

void Player::OnCollision(Collider* other)
{
	// ブロックとの衝突判定
	if (Block* block = dynamic_cast<Block*>(other)) {

	}
}

void Player::SaveToJson() {
	json j;

	// なんか追加する場合こっから
	j["gravityAcceleration"] = { gravityAcceleration_ };
	j["jumpAcceleration"] = {jumpAcceleration};

	// ディレクトリを作成し、JSONファイルを保存
	std::filesystem::create_directories("resources/jsons/Parameters/");
	std::ofstream outFile("resources/jsons/Parameters/" + className_ + ".json");
	outFile << j.dump(4);
}

void Player::LoadFromJson() {
	std::ifstream inFile("resources/jsons/Parameters/" + className_ + ".json");
	if (!inFile.is_open()) {
		return; // JSONファイルがない場合は早期リターン
	}

	json j;
	inFile >> j;

	// 各種JSONから読み込み
	if (j.contains("gravityAcceleration") && j["gravityAcceleration"].is_array()) {
		gravityAcceleration_ = j["gravityAcceleration"][0];
	}
	if (j.contains("jumpVelocity") && j["jumpVelocity"].is_array()) {
		jumpAcceleration = j["jumpVelocity"][0];
	}
}
