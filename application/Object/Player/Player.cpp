#include "Player.h"

void Player::Init(const std::string className) {
	input_ = Input::GetInstance();

	BaseObject::Init(className);
	BaseObject::CreateModel("debug/Cube.obj");
	BaseObject::CreateCollider();
	BaseObject::SetObjColor({1.0f, 0.0f, 0.0f, 1.0f});

	// 初期位置の設定（一旦雑にここで）
	const int x = 1;
	const int y = 3;
	BaseObject::SetWorldPosition({x * MapChipField::kChipSize, y * -MapChipField::kChipSize, 0.0f});

	///
	///	各パラメーター初期化
	///		
	jumpVelocity_ = 0.3f; // ジャンプ初速
	gravity_ = -0.01f; // 重力

	// Jsonからパラメーターの読み込み
	LoadFromJson();
}

void Player::Update(MapChipField* mapChipField) {
	BaseObject::Update();

	///
	///	移動
	///

	Move();

	///
	///	ジャンプ
	/// 

	Jump();

	///
	///	重力の適用とブロックへの着地
	///

	ApplyGravity(mapChipField);



	///
	///	範囲内のブロックを反転する操作
	///

	InvertBlocksInArea(mapChipField);

#ifdef _DEBUG

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
			ImGui::DragFloat("ジャンプ初速", &jumpVelocity_, 0.01f);
			ImGui::DragFloat("重力", &gravity_, 0.001f);

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

void Player::Move() {
	// 現在の位置を取得
	Vector3 position = BaseObject::GetWorldPosition();
	// 移動速度の設定
	const float kMoveSpeed = 0.15f; // 要調整

	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState)) {
		// 左スティックの入力値を取得
		float leftStickX = joyState.Gamepad.sThumbLX;
		float leftStickY = joyState.Gamepad.sThumbLY;

		// デッドゾーンの設定
		const float deadZone = 4000.0f;

		if (abs(leftStickX) > deadZone || abs(leftStickY) > deadZone) {
			// スティックの値を正規化して移動速度を調整
			const float maxStickValue = 32767.0f;
			float moveX = (abs(leftStickX) > deadZone) ? (leftStickX / maxStickValue) * kMoveSpeed : 0.0f;
			float moveY = (abs(leftStickY) > deadZone) ? (leftStickY / maxStickValue) * kMoveSpeed : 0.0f;

			// 移動量を反映
			position.x += moveX;
			position.y += moveY;

			// 新しい位置を設定
			BaseObject::SetWorldPosition(position);
		}
	}

	///
	///	キーボード入力による移動（中間プレイ会に一時的に）
	///

	if (input_->PushKey(DIK_W))
		position.y += kMoveSpeed;
	if (input_->PushKey(DIK_S))
		position.y -= kMoveSpeed;
	if (input_->PushKey(DIK_A))
		position.x -= kMoveSpeed;
	if (input_->PushKey(DIK_D))
		position.x += kMoveSpeed;

	BaseObject::SetWorldPosition(position);
}

void Player::Jump() { 
	static bool wasAPressed = false; // 前フレームのボタン状態を記録
	XINPUT_STATE joyState;

	// ジャンプ可能かチェック
	if (!isJumping_ && isOnGround_) {
		///
		///	Aボタンを押したらジャンプ
		/// 
		if (input_->GetJoystickState(0, joyState)) {
			bool isAPressed = joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A;

			// ボタンが押された瞬間を検出
			if (isAPressed && !wasAPressed) {
				isJumping_ = true;
				velocity_.y = jumpVelocity_; // ジャンプ初速を設定
			}

			// 現在の状態を記録
			wasAPressed = isAPressed;
		}
	}
}

void Player::ApplyGravity(MapChipField* mapChipField) { 
	// Y方向速度に重力の適用
	velocity_.y += gravity_;

	// 現在位置の取得
	Vector3 position = BaseObject::GetWorldPosition();

	// ブロックとの衝突判定
	const auto blocks = mapChipField->GetBlocks(); // 全てのブロックを取得
	for (const auto& block : blocks) {
		// ブロックの位置とサイズを取得
		Vector3 blockPosition = block->GetWorldPosition();
		const float blockSize = MapChipField::kChipSize;

		// プレイヤーがブロックの上部に接触しているか確認
		if (position.x + kWidth / 2 > blockPosition.x - blockSize / 2 &&
			position.x - kWidth / 2 < blockPosition.x + blockSize / 2 &&
			position.y - kHeight / 2 <= blockPosition.y + blockSize / 2 &&
		    position.y - kHeight / 2 > blockPosition.y) {

			// ジャンプ中でない場合のみ落下を止めて位置を調整
			if (!isJumping_) {
				position.y = blockPosition.y + blockSize / 2 + kWidth / 2;
				velocity_.y = 0.0f; // 落下速度をリセット
				isOnGround_ = true; // 地面に接触している
			}

			isJumping_ = false;
			break;

		// 地面に着地していない場合
		} else {
			isOnGround_ = false; // 地面に接触していない
		}
	}

	// 更新後の位置を適用
	position.y += velocity_.y;
	BaseObject::SetWorldPositionY(position.y);
}

void Player::InvertBlocksInArea(MapChipField* mapChipField) {
	static bool wasRightShoulderPressed = false; // 前フレームのボタン状態を記録

	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState)) {
		bool isRightShoulderPressed = joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

		// ボタンが押された瞬間を検出
		if (isRightShoulderPressed && !wasRightShoulderPressed) {
			if (mapChipField) {
				// 現在の位置を取得
				Vector3 position = BaseObject::GetWorldPosition();
				// 範囲内のブロックの反転を行う
				mapChipField->InvertBlocksInArea(position);

				// 挟み込んだブロックの反転処理
				/*mapChipField->InvertBlocksWithCapture();*/
			}
		}

		// 現在の状態を記録
		wasRightShoulderPressed = isRightShoulderPressed;
	}

	///
	///	キーボード入力によるブロック反転（中間プレイ会に一時的に）
	///

	static bool wasSpacePressed = false;
	bool isSpacePressed = input_->TriggerKey(DIK_SPACE);
	if (isSpacePressed && !wasSpacePressed) {
		if (mapChipField) {
			// 現在の位置を取得
			Vector3 position = BaseObject::GetWorldPosition();
			// 範囲内のブロックの反転を行う
			mapChipField->InvertBlocksInArea(position);
		}
	}

	// 現在の状態を記録
	wasSpacePressed = isSpacePressed;
}

void Player::SaveToJson() {
	json j;

	// なんか追加する場合こっから
	j["jumpVelocity"] = {jumpVelocity_};
	j["gravity"] = {gravity_};

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
	if (j.contains("jumpVelocity") && j["jumpVelocity"].is_array()) {
		jumpVelocity_ = j["jumpVelocity"][0];
	}
	if (j.contains("gravity") && j["gravity"].is_array()) {
		gravity_ = j["gravity"][0];
	}
}
