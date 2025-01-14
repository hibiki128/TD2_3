#include "Player.h"

void Player::Init(const std::string className)
{
	input_ = Input::GetInstance();

	BaseObject::Init(className);
	BaseObject::CreateModel("debug/Cube.obj");
	BaseObject::CreateCollider();
	BaseObject::SetObjColor({ 1.0f, 0.0f, 0.0f, 1.0f });

	// 初期位置の設定（一旦雑にここで）
	const int x = 1;
	const int y = 6;
	BaseObject::SetWorldPosition({ x * MapChipField::kChipSize, y * -MapChipField::kChipSize, 0.0f });
}

void Player::Update(MapChipField* mapChipField)
{
	BaseObject::Update();

	///
	///	移動
	/// 
	
	Move();

	///
	///	範囲内のブロックを反転
	/// 
	
	InvertBlocksInArea(mapChipField);
}

void Player::Draw(const ViewProjection& viewProjection)
{
	BaseObject::Draw(viewProjection);
}

void Player::Move()
{
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

void Player::InvertBlocksInArea(MapChipField* mapChipField)
{
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
