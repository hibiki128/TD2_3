#include "UIObject.h"

// C++
#include <cmath>

void UIObject::Init() {
	input_ = Input::GetInstance();

	// 本オブジェクト
	objectBook_ = std::make_unique<BaseObject>();
	objectBook_->Init("objectBook");
	objectBook_->CreateModel("game/openBook.obj");
	objectBook_->SetTexture("game/openBook.png");

	// 栞（操作説明）オブジェクト
	objectUI_ = std::make_unique<BaseObject>();
	objectUI_->Init("objectUI");
	objectUI_->CreateModel("game/UI.obj");
	objectUI_->SetTexture("game/ui.png");

	// Lスティックオブジェクト
	objectL_ = std::make_unique<BaseObject>();
	objectL_->Init("objectL");
	objectL_->CreateModel("game/L.obj");
	objectL_->SetTexture("game/L.png");

	initLstickPos_ = objectL_->GetTransform().translation_;

	// Aボタンオブジェクト
	objectA_ = std::make_unique<BaseObject>();
	objectA_->Init("objectA");
	objectA_->CreateModel("game/A.obj");
	objectA_->SetTexture("game/A.png");

	// RBボタンオブジェクト
	objectR_ = std::make_unique<BaseObject>();
	objectR_->Init("objectR");
	objectR_->CreateModel("game/R.obj");
	objectR_->SetTexture("game/R.png");
}

void UIObject::Update() {
	// パッド入力による反応
	InputReaction();

	objectBook_->Update();
	objectUI_->Update();
	objectL_->Update();
	objectA_->Update();
	objectR_->Update();
}

void UIObject::Draw(const ViewProjection& viewProjection) {
	objectBook_->Draw(viewProjection);
	objectUI_->Draw(viewProjection);
	objectL_->Draw(viewProjection);
	objectA_->Draw(viewProjection);
	objectR_->Draw(viewProjection);
}

void UIObject::DebugImGui()
{
	objectBook_->DebugImGui();
	objectUI_->DebugImGui();
	objectL_->DebugImGui();
	objectA_->DebugImGui();
	objectR_->DebugImGui();
}

void UIObject::InputReaction()
{
	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState)) {
		///
		///	Lスティックオブジェクトを動かす
		/// 

		// 左スティックの入力値を取得
		float leftStickX = static_cast<float>(joyState.Gamepad.sThumbLX) / 32768.0f;
		float leftStickY = static_cast<float>(joyState.Gamepad.sThumbLY) / 32768.0f;

		// デッドゾーン処理
		const float deadZone = 0.2f;
		if (std::abs(leftStickX) < deadZone) leftStickX = 0.0f;
		if (std::abs(leftStickY) < deadZone) leftStickY = 0.0f;


		// 入力がある場合のみ処理
		if (leftStickX != 0.0f || leftStickY != 0.0f) {
			// 初期回転角を考慮した変換
			float rotationAngle = objectL_->GetTransform().rotation_.z;
			float cosAngle = std::cos(rotationAngle);
			float sinAngle = std::sin(rotationAngle);

			// 入力値を回転角で変換
			float adjustedX = leftStickX * cosAngle - leftStickY * sinAngle;
			float adjustedY = leftStickX * sinAngle + leftStickY * cosAngle;

			// 移動範囲の制限
			const float moveRange = 0.5f;
			Vector3 newPosition = initLstickPos_;
			newPosition.x += adjustedX * moveRange;
			newPosition.y += adjustedY * moveRange;

			// Lオブジェクトの位置を更新
			objectL_->SetWorldPosition(newPosition);

			// 入力が無い場合は初期位置に戻す
		} else {
			objectL_->SetWorldPosition(initLstickPos_);
		}

		///
		///	Aボタンが押されている間は色を濃くする
		/// 
		
		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
			objectA_->SetObjColor({ 0.3f, 0.3f, 0.3f, 1.0f });
		} else {
			objectA_->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}

		///
		///	 RBボタンが押されている間は色を濃くする
		/// 
		
		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			objectR_->SetObjColor({ 0.3f, 0.3f, 0.3f, 1.0f });
		} else {
			objectR_->SetObjColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
	}
}
