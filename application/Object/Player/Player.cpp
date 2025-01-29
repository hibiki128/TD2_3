#define NOMINMAX
#include "Player.h"

// Engine
#include "myEngine/3d/line/DrawLine3D.h"
#include "math/myMath.h"
#include"Audio.h"
#include"myEngine/Frame/Frame.h"

void Player::Init(const std::string className) {
	input_ = Input::GetInstance();

	BaseObject::Init(className);
	BaseObject::CreateModel("game/Player.obj");
	BaseObject::SetTexture("debug/white1x1.png"); // 白状態のプレイヤーテクスチャを設定
	BaseObject::CreateCollider();
	Collider::SetVisible(false);

	///
	///	各パラメーター初期化
	///

	gravityAcceleration_ = -0.01f; // 重力
	jumpAcceleration_ = 0.3f;      // ジャンプ初速

	xInvertRange_ = 3;
	yInvertRange_ = 3;

	///
	///	その他
	///

	// SquareTransition初期化
	squareTransition_ = std::make_unique<SquareTransition>();
	squareTransition_->Initialize();

	// プレイヤー反転範囲スプライト生成
	spritePlayerArea_ = std::make_unique<Sprite>();
	spritePlayerArea_->Initialize(
		"game/playerArea.png",
		{0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f}
		);
	spritePlayerArea_->SetSize({165.0f, 165.0f});

	// Jsonからパラメーターの読み込み
	LoadFromJson();

	jumpSE_ = Audio::GetInstance()->LoadWave("player/playerJump.wav");
	landingSE_ = Audio::GetInstance()->LoadWave("player/playerLanding.wav");
	walkSE_ = Audio::GetInstance()->LoadWave("player/playerWalk.wav");
	gravitySE_ = Audio::GetInstance()->LoadWave("action/inversionGravity.wav");
	inversionSE_ = Audio::GetInstance()->LoadWave("action/inversion.wav");
}

void Player::Update(MapChipField* mapChipField) {
	///
	///	毎フレーム更新処理
	///

	BaseObject::Update();
	squareTransition_->Update();

	// ブロック反転時、アニメーションが終わるまでを判定（反転中はプレイヤーが動かないようにするため）
	if (isInverting_) {
		invertTimer_ += kDeltaTime;
		if (invertTimer_ >= invertDuration_) {
			isInverting_ = false; // 反転が終了したことを示す
			invertTimer_ = 0.0f;  // タイマーリセット
		}
	}

	///
	///	毎フレーム初期化処理
	///

	mapChipField_ = mapChipField;

	// 接地しているか天井に接触した際にはY方向速度をリセット
	if (collisionMapInfo_.hittingGround_) {
		velocity_.y = 0.0f;

	}
	else if (collisionMapInfo_.hittingCeiling_) {
		velocity_.y = 0.0f;
	}

	// 各種瞬間判定フラグをリセット
	isJumpOccurred_ = false;
	isBlockInversionOccurred_ = false;
	isResetOccurred_ = false;
	isGravityReversedOccurred_ = false;

	///
	///	入力操作
	///

	HandleInput();

	///
	///	重力を常に受ける
	///

	if (!isInverting_) { // ブロック反転中には重力を加算しない
		if (isGravityReversed_) {  // 重力反転中
			velocity_.y -= gravityAcceleration_; // 上向きに重力をかける (逆)
		}
		else { // 通常重力
			velocity_.y += gravityAcceleration_; // 下向きに重力をかける（順）
		}
	}

	///
	///	全てのブロックとの衝突判定とプレイヤーの押し戻し
	///

	CheckCollisionAndResolve();

#ifdef _DEBUG
	ImGui::Begin("player");

	if (ImGui::BeginTabBar(className_.c_str())) {
		if (ImGui::BeginTabItem("デバッグ")) {

			/*ImGui::DragFloat3("velocity", &velocity_.x);*/

			ImGui::Text("hittingGround : %d", collisionMapInfo_.hittingGround_);
			ImGui::Text("hittingCeiling : %d", collisionMapInfo_.hittingCeiling_);
			ImGui::Text("hittingLeft : %d", collisionMapInfo_.hittingLeft_);
			ImGui::Text("hittingRight : %d", collisionMapInfo_.hittingRight_);
			ImGui::Text("isOverlapping : %d", collisionMapInfo_.isOverlapping_);

			/*ImGui::Checkbox("ブロック反転中", &isInverting_);
			ImGui::Checkbox("重力反転中", &isGravityReversed_);*/

			/*ImGui::Text("TransitionStatus : %d", squareTransition_->GetCurrentStatus());
			ImGui::Text("TransitionIsFinished : %d", squareTransition_->IsFinished());*/

			bool flag[5] = { false };
			flag[0] = IsJumpOccurred();
			flag[1] = IsBlockInversionOccurred();
			flag[2] = IsResetOccurred();
			flag[3] = IsGravityReversedOccurred();
			flag[4] = IsLandedOccurred();

			ImGui::Checkbox("ジャンプした瞬間", &flag[0]);
			ImGui::Checkbox("ブロック反転した瞬間", &flag[1]);
			ImGui::Checkbox("リセットした瞬間", &flag[2]);
			ImGui::Checkbox("重力反転した瞬間", &flag[3]);
			ImGui::Checkbox("着地した瞬間", &flag[4]);

			if (colorState_ == ColorState::White) {
				ImGui::Text("現在の色 : 白");
			}
			else if (colorState_ == ColorState::Black) {
				ImGui::Text("現在の色 : 黒");
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
#endif
}

void Player::Draw(const ViewProjection& viewProjection) {
	BaseObject::Draw(viewProjection);

	// 反転可能範囲を描画
	/*DrawInvertArea();*/
}

void Player::DrawSprite(const ViewProjection& viewProjection) { 
	// プレイヤーのワールド座標をスクリーン座標に変換してspritePlayerAreaの位置をセット
	InvertAreaSpriteToPlayerPosition(viewProjection);
	// 現在の反転可能範囲の数値によってspritePlayerAreaのサイズを変更
	InvertAreaSpriteAdjust();

	// プレイヤー反転可能範囲の描画
	spritePlayerArea_->Draw();

	// リセット時トランジションスプライトの描画
	squareTransition_->Draw(); 
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
			ImGui::DragFloat("ジャンプ初速", &jumpAcceleration_, 0.01f);

			ImGui::DragInt("X方向反転範囲", &xInvertRange_, 2, 1, 101);
			ImGui::DragInt("Y方向反転範囲", &yInvertRange_, 2, 1, 101);

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

bool Player::IsGoalReached() {
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
		if (corner.x >= goalLeft && corner.x <= goalRight && corner.y >= goalBottom && corner.y <= goalTop) {
			return true; // 4つ角のどれかが触れていたらtrue
		}
	}

	return false;
}

void Player::HandleInput() {
#pragma region ゲームパッド入力
	// 前フレームの押下状態を保存
	static bool wasPressedA = false; // Aボタン
	static bool wasPressedRB = false; // RBボタン
	static bool wasPressedLB = false; // LBボタン

	// ブロック反転クールタイムの減少
	if (blockInvertCooldown_ > 0.0f) {
		blockInvertCooldown_ -= kDeltaTime;
	}

	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState)) {

		///
		///	左右移動入力
		///

		if (!isInverting_) { // ブロック反転中には移動できない
			// 左スティックの入力値を取得
			float leftStickX = joyState.Gamepad.sThumbLX;
			// デッドゾーンの設定
			const float deadZone = 4000.0f;

			if (abs(leftStickX) > deadZone) {
				const float maxStickValue = 32767.0f;
				float moveX = (abs(leftStickX) > deadZone) ? (leftStickX / maxStickValue) * kMoveSpeed : 0.0f;

				// 移動量を反映
				velocity_.x = moveX;
			}
		}

		///
		///	ジャンプ入力
		///

		bool isPressedA = joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A;

		// Aボタンが押された瞬間のみ
		if (isPressedA && !wasPressedA) {
			if (isGravityReversed_) { // 重力反転中
				// 天井にいる場合のみ
				if (collisionMapInfo_.hittingCeiling_) {
					velocity_.y = -jumpAcceleration_; // 下向き (逆)

					// ジャンプしたことを記録（SE・エフェクト用）
					isJumpOccurred_ = true;
				}
			}
			else {
				// 地面にいる場合のみ
				if (collisionMapInfo_.hittingGround_) {
					velocity_.y = jumpAcceleration_; // 上向き (順)

					// ジャンプしたことを記録（SE・エフェクト用）
					isJumpOccurred_ = true;
				}
			}
		}

		// 前フレームの状態を記録
		wasPressedA = isPressedA;

		///
		///	範囲内のブロック反転入力
		///

		bool isPressedRB = joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

		// RBボタンが押された瞬間のみ
		if (isPressedRB && !wasPressedRB && blockInvertCooldown_ <= 0.0f) { // クールタイム中には反転できない
			if (!isInverting_ && !collisionMapInfo_.isOverlapping_) { // ブロック反転中には反転できない && ブロックに埋まっていたら反転できない
				if (mapChipField_) {
					// 現在の位置を取得
					Vector3 position = BaseObject::GetWorldPosition();

					// 範囲内にブロックが1つでも存在しているかを判定する
					if (mapChipField_->HasBlockInArea(position, xInvertRange_, yInvertRange_)) {
						// 範囲内のブロックの反転を行う
						mapChipField_->InvertBlocksInArea(position, xInvertRange_, yInvertRange_);
						// 反転中であることを記録する
						isInverting_ = true;
						// ブロック反転クールタイムを設定
						blockInvertCooldown_ = kBlockInvertCooldownTime;

						///
						/// 重力ブロックが範囲内に見つかった場合、プレイヤーの重力を反転する
						/// 
						if (mapChipField_->HasGravityBlockInArea(position, xInvertRange_, yInvertRange_)) {
							isGravityReversed_ = !isGravityReversed_;


							// 重力反転したことを記録（SE・エフェクト用）
							isGravityReversedOccurred_ = true;
						}

						///
						///	プレイヤー色反転ブロックが範囲内に見つかった場合、プレイヤーの色を反転する
						/// 
						if (mapChipField_->HasColorChangeBlockInArea(position, xInvertRange_, yInvertRange_)) {
							// 現在が白の場合、テクスチャと色状態を黒に変更
							if (colorState_ == ColorState::White) {
								this->SetTexture("debug/black1x1.png");
								colorState_ = ColorState::Black;
								// 現在が黒の場合、テクスチャと色状態を白に変更
							}
							else if (colorState_ == ColorState::Black) {
								this->SetTexture("debug/white1x1.png");
								colorState_ = ColorState::White;
							}
						}

						// ブロック反転したことを記録（SE・エフェクト用）
						isBlockInversionOccurred_ = true;
					}
				}
			}
		}

		// 前フレームの状態を記録
		wasPressedRB = isPressedRB;

		///
		///	リセット
		///

		bool isPressedLB = joyState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;

		if (isPressedLB && !wasPressedLB) {
			// トランジション中には押せないようにする
			if (squareTransition_->IsFinished()) {
				// SquareInを開始する
				squareTransition_->Start(SquareTransition::Status::SquareIn, kResetTransitionTime);

				// リセットしたことを記録（SE・エフェクト用）
				isResetOccurred_ = true;
			}
		}

		// 前フレームの状態を記録
		wasPressedLB = isPressedLB;
	}
#pragma endregion

#pragma region キーボード入力
	///
	///	左右移動入力
	///

	if (!isInverting_) { // ブロック反転中には移動できない
		if (input_->PushKey(DIK_A)) {
			velocity_.x = -kMoveSpeed;
		}
		if (input_->PushKey(DIK_D)) {
			velocity_.x = kMoveSpeed;
		}
	}

	///
	///	ジャンプ入力
	///

	if (input_->TriggerKey(DIK_W)) {
		if (isGravityReversed_) { // 重力反転中
			// 天井にいる場合のみ
			if (collisionMapInfo_.hittingCeiling_) {
				velocity_.y = -jumpAcceleration_; // 下向き (逆)

				// ジャンプしたことを記録（SE・エフェクト用）
				isJumpOccurred_ = true;
			}
		}
		else {
			// 地面にいる場合のみ
			if (collisionMapInfo_.hittingGround_) {
				velocity_.y = jumpAcceleration_; // 上向き (順)

				// ジャンプしたことを記録（SE・エフェクト用）
				isJumpOccurred_ = true;
			}
		}
	}

	///
	///	範囲内のブロック反転入力
	///

	if (input_->TriggerKey(DIK_SPACE) && blockInvertCooldown_ <= 0.0f) { // クールタイム中には反転できない
		if (!isInverting_ && !collisionMapInfo_.isOverlapping_) { // ブロック反転中には反転できない && ブロックに埋まっていたら反転できない
			if (mapChipField_) {
				// 現在の位置を取得
				Vector3 position = BaseObject::GetWorldPosition();

				// 範囲内にブロックが1つでも存在しているかを判定する
				if (mapChipField_->HasBlockInArea(position, xInvertRange_, yInvertRange_)) {
					// 範囲内のブロックの反転を行う
					mapChipField_->InvertBlocksInArea(position, xInvertRange_, yInvertRange_);
					// 反転中であることを記録する
					isInverting_ = true;
					// ブロック反転クールタイムを設定
					blockInvertCooldown_ = kBlockInvertCooldownTime;

					///
					/// 重力ブロックが範囲内に見つかった場合、プレイヤーの重力を反転する
					/// 
					if (mapChipField_->HasGravityBlockInArea(position, xInvertRange_, yInvertRange_)) {
						isGravityReversed_ = !isGravityReversed_;


						// 重力反転したことを記録（SE・エフェクト用）
						isGravityReversedOccurred_ = true;
					}

					///
					///	プレイヤー色反転ブロックが範囲内に見つかった場合、プレイヤーの色を反転する
					/// 
					if (mapChipField_->HasColorChangeBlockInArea(position, xInvertRange_, yInvertRange_)) {
						// 現在が白の場合、テクスチャと色状態を黒に変更
						if (colorState_ == ColorState::White) {
							this->SetTexture("debug/black1x1.png");
							colorState_ = ColorState::Black;
							// 現在が黒の場合、テクスチャと色状態を白に変更
						}
						else if (colorState_ == ColorState::Black) {
							this->SetTexture("debug/white1x1.png");
							colorState_ = ColorState::White;
						}
					}

					// ブロック反転したことを記録（SE・エフェクト用）
					isBlockInversionOccurred_ = true;
				}
			}
		}
	}

	///
	///	リセット
	///

	if (input_->TriggerKey(DIK_R)) {
		// トランジション中には押せないようにする
		if (squareTransition_->IsFinished()) {
			// SquareInを開始する
			squareTransition_->Start(SquareTransition::Status::SquareIn, kResetTransitionTime);

			// リセットしたことを記録（SE・エフェクト用）
			isResetOccurred_ = true;
		}
	}

#pragma endregion
}

bool Player::IsLandedOccurred() {
	if (!isInverting_) { // 足元のブロックを反転させた際にも反応してしまうのを防止
		// 重力が通常の場合
		if (!isGravityReversed_) {
			bool currentHittingGround = collisionMapInfo_.hittingGround_;

			// 着地した瞬間のみを判定
			if (!prevHittingGround_ && currentHittingGround) {
				prevHittingGround_ = currentHittingGround;
				return true;
			}

			prevHittingGround_ = currentHittingGround;

			// 重力が逆の場合
		}
		else {
			bool currentHittingGround = collisionMapInfo_.hittingCeiling_;

			// 着地した瞬間のみを判定
			if (!prevHittingGround_ && currentHittingGround) {
				prevHittingGround_ = currentHittingGround;
				return true;
			}

			prevHittingGround_ = currentHittingGround;
		}
	}

	return false;
}

bool Player::IsWalking()
{
	if (input_->PushKey(DIK_D) || input_->PushKey(DIK_A)) {
		isWalking_ = true;
	}
	else {
		isWalking_ = false;
	}
	if (isWalking_) {
		if (walkSEcoolTime_ < 0) {
			walkSEcoolTime_ = 0.5f;
			return true;
		}
		walkSEcoolTime_ -= Frame::DeltaTime();
	}
	return false;
}

void Player::Reset() {
	///
	///	メモ : SquareInが呼び出されたら終了次第、リセットとSquareOutが開始する
	///

	// Rキー押下時にSquareInが開始するので、終了したらリセット処理が行われる
	if (squareTransition_->IsFinished() && squareTransition_->GetCurrentStatus() == SquareTransition::Status::SquareIn) {
		///
		///	各種リセット処理
		/// 

		// プレイヤーの位置をリセット
		this->transform_.translation_ = mapChipField_->GetPlayerInitialPosition();
		// プレイヤーの速度をリセット
		this->velocity_ = { 0.0f, 0.0f, 0.0f };
		// プレイヤーの重力状態をリセット
		isGravityReversed_ = false;
		// プレイヤーの色状態をリセット（とりあえずデフォルトを白としておく）
		this->SetTexture("debug/white1x1.png");
		colorState_ = ColorState::White;

		// マップのリセット
		mapChipField_->ResetMapChip();
		// マップの所持する重力状態をリセット
		mapChipField_->SetIsGravityReversed(false);


		// SquareOutを開始する
		squareTransition_->Start(SquareTransition::Status::SquareOut, kResetTransitionTime);
	}

	// プレイヤーが画面外へ落下した際にもリセット
	if (squareTransition_->IsFinished()) {
		if (this->transform_.translation_.y < -30.0f || this->transform_.translation_.y > 30.0f) { // リセット判定の座標を一旦適当に設定
			// SquareInを開始する
			squareTransition_->Start(SquareTransition::Status::SquareIn, kResetTransitionTime);
		}
	}
}

void Player::PlaySE()
{
	Audio* audio = Audio::GetInstance();
	if (IsJumpOccurred()) {
		audio->PlayWave(jumpSE_, 0.1f);
	}
	if (IsLandedOccurred()) {
		audio->PlayWave(landingSE_, 0.1f);
	}
	if (IsWalking()) {
		audio->PlayWave(walkSE_, 0.1f);
	}
	if (IsBlockInversionOccurred()) {
		audio->PlayWave(inversionSE_, 0.1f);
	}
	if (IsGravityReversedOccurred()) {
		audio->PlayWave(gravitySE_, 0.1f);
	}
}

void Player::CheckCollisionAndResolve() {
	/// X移動
	BaseObject::transform_.translation_.x += velocity_.x;

	/// 衝突判定
	CollisionMapInfo collisionMapInfoX = GetMapCollisionInfo();

	/// 押し戻し
	if (collisionMapInfoX.hittingLeft_) {
		Vector3 blockPosition = collisionMapInfoX.blockX->GetWorldPosition();
		float blockRight = blockPosition.x + MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.x = blockRight + kWidth / 2 + kBlank; // 左側に衝突した場合、右に押し戻し
	}
	else if (collisionMapInfoX.hittingRight_) {
		Vector3 blockPosition = collisionMapInfoX.blockX->GetWorldPosition();
		float blockLeft = blockPosition.x - MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.x = blockLeft - kWidth / 2 - kBlank; // 右側に衝突した場合、左に押し戻し
	}

	/// Y移動
	if (!isInverting_) { // ブロック反転中には移動しない
		BaseObject::transform_.translation_.y += velocity_.y;
	}

	/// 衝突判定
	CollisionMapInfo collisionMapInfoY = GetMapCollisionInfo();

	/// 押し戻し
	if (collisionMapInfoY.hittingGround_) {
		Vector3 blockPosition = collisionMapInfoY.blockY->GetWorldPosition();
		float blockBottom = blockPosition.y + MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.y = blockBottom + kHeight / 2 + kBlank; // 地面の位置に押し戻し
	}
	else if (collisionMapInfoY.hittingCeiling_) {
		Vector3 blockPosition = collisionMapInfoY.blockY->GetWorldPosition();
		float blockTop = blockPosition.y - MapChipField::kChipSize / 2;
		BaseObject::transform_.translation_.y = blockTop - kHeight / 2 - kBlank; // 天井の位置に押し戻し
	}

	/// 衝突判定を格納
	collisionMapInfo_.hittingGround_ = collisionMapInfoY.hittingGround_;
	collisionMapInfo_.hittingCeiling_ = collisionMapInfoY.hittingCeiling_;

	collisionMapInfo_.hittingLeft_ = collisionMapInfoX.hittingLeft_;
	collisionMapInfo_.hittingRight_ = collisionMapInfoX.hittingRight_;

	collisionMapInfo_.isOverlapping_ = collisionMapInfoX.isOverlapping_;
	collisionMapInfo_.isOverlapping_ = collisionMapInfoY.isOverlapping_;

	/// 速度リセット
	velocity_.x = 0.0f;
	/*velocity_.y = 0.0f;*/
}

void Player::DrawInvertArea() {
	// プレイヤーの位置を取得
	Vector3 playerPositon = this->transform_.translation_;

	// AABBの範囲を計算
	float minX = playerPositon.x - xInvertRange_;
	float maxX = playerPositon.x + xInvertRange_;
	float minY = playerPositon.y - yInvertRange_;
	float maxY = playerPositon.y + yInvertRange_;
	float minZ = playerPositon.z - 1.0f;
	float maxZ = playerPositon.z + 1.0f;

	// AABBの頂点を計算
	std::vector<Vector3> vertices = {
		{minX, minY, minZ},
		{maxX, minY, minZ},
		{maxX, maxY, minZ},
		{minX, maxY, minZ}, // 底面
		{minX, minY, maxZ},
		{maxX, minY, maxZ},
		{maxX, maxY, maxZ},
		{minX, maxY, maxZ}  // 上面
	};

	// AABBのエッジリスト
	std::vector<std::pair<int, int>> edges = {
		{0, 1},
		{1, 2},
		{2, 3},
		{3, 0}, // 底面
		{4, 5},
		{5, 6},
		{6, 7},
		{7, 4}, // 上面
		{0, 4},
		{1, 5},
		{2, 6},
		{3, 7}  // 側面
	};

	// エッジを描画
	for (const auto& edge : edges) {
		DrawLine3D::GetInstance()->SetPoints(vertices[edge.first], vertices[edge.second], { 1.0f, 1.0f, 1.0f, 1.0f });
	}
}

void Player::InvertAreaSpriteToPlayerPosition(const ViewProjection& viewProjection) {
	// spritePlayerAreaにプレイヤーのワールド座標を設定
	Vector3 playerWorldPosition = this->GetWorldPosition();

	// ビューポート行列を作成
	Matrix4x4 matViewport = MakeViewPortMatrix(0.0f, 0.0f, WinApp::kClientWidth, WinApp::kClientHeight, 0, 1);

	// ビュー行列とプロジェクション行列を合成
	Matrix4x4 matViewProjection = viewProjection.matView_ * viewProjection.matProjection_;
	Matrix4x4 matViewProjecitonViewport = matViewProjection * matViewport;

	// プレイヤーのワールド座標をスクリーン座標に変換
	Vector3 screenPosition = Transformation(playerWorldPosition, matViewProjecitonViewport);

	spritePlayerArea_->SetPosition({screenPosition.x, screenPosition.y});
}

void Player::InvertAreaSpriteAdjust()
{
	Vector2 spriteSize;

	// 反転可能範囲の数値によってスプライトのサイズを設定する
	const float sizes[] = { 0.0f, 55.0f, 0.0f, 165.0f, 0.0f, 275.0f, 0.0f, 383.0f, 0.0f, 490.0f, 0.0f, 598.0f }; // 目視で合わせた各サイズ

	// xサイズ変更
	if (xInvertRange_ >= 1 && xInvertRange_ <= 11 && xInvertRange_ % 2 == 1)
	{
		spriteSize.x = sizes[xInvertRange_];
	}

	// yサイズ変更
	if (yInvertRange_ >= 1 && yInvertRange_ <= 11 && yInvertRange_ % 2 == 1)
	{
		spriteSize.y = sizes[yInvertRange_];
	}

	spritePlayerArea_->SetSize(spriteSize);
}

Player::CollisionMapInfo Player::GetMapCollisionInfo() {
	CollisionMapInfo info;

	// 現在位置の取得
	Vector3 position = this->transform_.translation_;

	// 重なり判定のオフセット（プレイヤーの実際のサイズよりも少し減らした値で判定）
	const float overlapOffsetX = (kWidth / 2) - 0.02f;
	const float overlapOffsetY = (kHeight / 2) - 0.02f;

	// 重なり判定用の4点
	Vector3 checkPoints[4] = {
		{position.x - overlapOffsetX, position.y + overlapOffsetY, position.z}, // 左上
		{position.x + overlapOffsetX, position.y + overlapOffsetY, position.z}, // 右上
		{position.x - overlapOffsetX, position.y - overlapOffsetY, position.z}, // 左下
		{position.x + overlapOffsetX, position.y - overlapOffsetY, position.z}, // 右下
	};

	// プレイヤーの4つの角を計算
	Vector3 corners[4] = {
		{position.x - kWidth / 2, position.y + kHeight / 2, position.z}, // 左上
		{position.x + kWidth / 2, position.y + kHeight / 2, position.z}, // 右上
		{position.x - kWidth / 2, position.y - kHeight / 2, position.z}, // 左下
		{position.x + kWidth / 2, position.y - kHeight / 2, position.z}  // 右下
	};

	// 中心左と中心右の点を計算
	Vector3 centerLeft = { position.x - kWidth / 2, position.y, position.z };  // 中心左
	Vector3 centerRight = { position.x + kWidth / 2, position.y, position.z }; // 中心右

	// 全てのブロックを取得
	const auto blocks = mapChipField_->GetBlocks();
	const float blockSize = MapChipField::kChipSize;

	// 全てのブロックとの衝突判定
	for (const auto& block : blocks) {
		// プレイヤーとブロックの色が同じ場合には判定を取らない
		if (this->colorState_ == ColorState::White && block->type_ == Block::ChipType::White) { // プレイヤーが白状態で、白ブロックの場合
			continue;
		}
		else if (this->colorState_ == ColorState::Black && block->type_ == Block::ChipType::Black) { // プレイヤーが黒状態で、黒ブロックの場合
			continue;
		}

		// ブロックの位置と範囲を計算
		Vector3 blockPosition = block->GetWorldPosition();
		float blockLeft = blockPosition.x - blockSize / 2;
		float blockRight = blockPosition.x + blockSize / 2;
		float blockTop = blockPosition.y + blockSize / 2;
		float blockBottom = blockPosition.y - blockSize / 2;

		// 重なり判定（プレイヤーの中心+-オフセットがブロックに接触しているか）
		for (int i = 0; i < 4; ++i) {
			if (checkPoints[i].x >= blockLeft && checkPoints[i].x <= blockRight &&
				checkPoints[i].y >= blockBottom && checkPoints[i].y < blockTop) {
				info.isOverlapping_ = true;
				break;
			}
		}

		// プレイヤーとブロックの色が同じ場合には上下左右の判定を取らない（押し戻しを行わないため）
		if (this->colorState_ == ColorState::White && block->type_ == Block::ChipType::White) { // プレイヤーが白状態で、白ブロックの場合
			continue;
		} else if (this->colorState_ == ColorState::Black && block->type_ == Block::ChipType::Black) { // プレイヤーが黒状態で、黒ブロックの場合
			continue;
		}

		// 各角の衝突を判定
		for (int i = 0; i < 4; ++i) {
			if (corners[i].x >= blockLeft && corners[i].x <= blockRight && corners[i].y >= blockBottom && corners[i].y <= blockTop) {
				// 上下判定
				if (i < 2) { // 左上・右上
					info.hittingCeiling_ = true;
					info.blockY = block; // Y方向で衝突したブロックを格納
				}
				else if (i >= 2) { // 左下・右下
					info.hittingGround_ = true;
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

		// 中心左の衝突判定
		if (centerLeft.x >= blockLeft && centerLeft.x <= blockRight && centerLeft.y >= blockBottom && centerLeft.y <= blockTop) {
			info.hittingLeft_ = true;
			info.blockX = block; // 中心左のX方向で衝突したブロックを格納
		}
		// 中心右の衝突判定
		if (centerRight.x >= blockLeft && centerRight.x <= blockRight && centerRight.y >= blockBottom && centerRight.y <= blockTop) {
			info.hittingRight_ = true;
			info.blockX = block; // 中心右のX方向で衝突したブロックを格納
		}
	}

	return info;
}

// void Player::OnCollision(Collider* other)
//{
//	// ブロックとの衝突判定
//	if (Block* block = dynamic_cast<Block*>(other)) {
//
//	}
// }

void Player::SaveToJson() {
	json j;

	// なんか追加する場合こっから
	j["gravityAcceleration"] = { gravityAcceleration_ };
	j["jumpAcceleration"] = { jumpAcceleration_ };

	j["xInvertRange"] = { xInvertRange_ };
	j["yInvertRange"] = { yInvertRange_ };

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
	if (j.contains("jumpAcceleration") && j["jumpAcceleration"].is_array()) {
		jumpAcceleration_ = j["jumpAcceleration"][0];
	}

	if (j.contains("xInvertRange") && j["xInvertRange"].is_array()) {
		xInvertRange_ = j["xInvertRange"][0];
	}
	if (j.contains("yInvertRange") && j["yInvertRange"].is_array()) {
		yInvertRange_ = j["yInvertRange"][0];
	}
}
