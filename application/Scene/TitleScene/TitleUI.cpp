#include "TitleUI.h"
#include"SpriteCommon.h"
#include"Easing.h"
#include"myEngine/Frame/Frame.h"


/// タイトルの仕様書**************************************************************************
/// https://docs.google.com/document/d/1HWlYKVP7claXDMJmXlaweO116KhdEbmu_mTf-6ORF1I/edit?tab=t.hs6t3nu61xqa#heading=h.jcw0f0b1h3x5
///*****************************************************************************************


void TitleUI::Init()
{
	/*start_ = std::make_unique<BaseObject>();
	start_->Init("start");
	start_->CreateModel("title/titleStart.obj");
	start_->SetTexture("title/title.png");*/

	title_ = std::make_unique<BaseObject>();
	title_->Init("title");
	title_->CreateModel("title/title.obj");
	title_->SetTexture("title/title.png");

	t_ = 0.0f;
	startTime_ = 1.5f;

	input_ = Input::GetInstance();

	// 本オブジェクト
	objectBook_ = std::make_unique<BaseObject>();
	objectBook_->Init("title_objectBook");
	objectBook_->CreateModel("game/openBook.obj");
	objectBook_->SetTexture("game/openBook.png");

	// 栞（操作説明）オブジェクト
	objectUI_ = std::make_unique<BaseObject>();
	objectUI_->Init("title_objectUI");
	objectUI_->CreateModel("game/UI.obj");
	objectUI_->SetTexture("game/ui.png");

	// Lスティックオブジェクト
	objectL_ = std::make_unique<BaseObject>();
	objectL_->Init("title_objectL");
	objectL_->CreateModel("game/L.obj");
	objectL_->SetTexture("game/L.png");

	initLstickPos_ = objectL_->GetTransform().translation_;

	// Aボタンオブジェクト
	objectA_ = std::make_unique<BaseObject>();
	objectA_->Init("title_objectA");
	objectA_->CreateModel("game/A.obj");
	objectA_->SetTexture("game/A.png");

	// RBボタンオブジェクト
	objectR_ = std::make_unique<BaseObject>();
	objectR_->Init("title_objectR");
	objectR_->CreateModel("game/R.obj");
	objectR_->SetTexture("game/R.png");

	// 各ボタンオブジェクトを、栞オブジェクトと親子付け
	objectL_->SetParent(&objectUI_->GetWorldTransform());
	objectA_->SetParent(&objectUI_->GetWorldTransform());
	objectR_->SetParent(&objectUI_->GetWorldTransform());
}

void TitleUI::Update()
{
	/// 各オブジェクト更新
	title_->Update();
	//start_->Update();
	TextMove();

	// パッド入力による反応
	InputReaction();

	objectBook_->Update();
	objectUI_->Update();
	objectL_->Update();
	objectA_->Update();
	objectR_->Update();
}

void TitleUI::Draw(const ViewProjection& vp)
{
	/// 各オブジェクト描画
	title_->Draw(vp);
	//start_->Draw(vp);

	objectBook_->Draw(vp);
	objectUI_->Draw(vp);
	objectL_->Draw(vp);
	objectA_->Draw(vp);
	objectR_->Draw(vp);
}

void TitleUI::Debug()
{
	/// 各オブジェクトデバッグ描画
	title_->DebugImGui();
	//start_->DebugImGui();

	objectBook_->DebugImGui();
	objectUI_->DebugImGui();
	objectL_->DebugImGui();
	objectA_->DebugImGui();
	objectR_->DebugImGui();
}

void TitleUI::TextMove()
{
	const float startPos = 16.0f;
	const float endPos = 4.0f;
	const float easeTMax = 3.0f;

	if (startTime_ <= 0) {
		if (t_ < easeTMax) {
			t_ += Frame::DeltaTime();
		}
		else {
			t_ = easeTMax;
		}
		startTime_ = 0;
	}
	else {
		startTime_ -= Frame::DeltaTime();
	}

	title_->SetWorldPositionY(EaseOutBounce<float>(startPos, endPos, t_, easeTMax));

}

void TitleUI::InputReaction()
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
		if (std::abs(leftStickX) < deadZone)
			leftStickX = 0.0f;
		if (std::abs(leftStickY) < deadZone)
			leftStickY = 0.0f;

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
			const float moveRange = 0.3f;
			Vector3 newPosition = initLstickPos_;
			newPosition.x += adjustedX * moveRange;
			newPosition.z += adjustedY * moveRange;

			// Lオブジェクトの位置を更新
			objectL_->SetWorldPosition(newPosition);

			// 入力が無い場合は初期位置に戻す
		} else {
			objectL_->SetWorldPosition(initLstickPos_);
		}

		/*押されたボタンの色の補間処理*/

		const float kDeltaTime = 1.0f / 60.0f;
		const float kLerpSpeed = 30.0f; // 補間速度

		///
		///	Aボタンが押されている間は色を濃くする
		///

		static Vector4 currentColorA(1.0f, 1.0f, 1.0f, 1.0f);
		Vector4 targetColorA;

		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
			targetColorA = {0.3f, 0.3f, 0.3f, 1.0f};
		} else {
			targetColorA = {1.0f, 1.0f, 1.0f, 1.0f};
		}
		// 現在の色を線形補間で更新
		currentColorA = Lerp(currentColorA, targetColorA, kLerpSpeed * kDeltaTime);
		objectA_->SetObjColor(currentColorA);

		///
		///	 RBボタンが押されている間は色を濃くする
		///

		static Vector4 currentColorR(1.0f, 1.0f, 1.0f, 1.0f);
		Vector4 targetColorR;

		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			targetColorR = {0.3f, 0.3f, 0.3f, 1.0f};
		} else {
			targetColorR = {1.0f, 1.0f, 1.0f, 1.0f};
		}
		// 現在の色を線形補間で更新
		currentColorR = Lerp(currentColorR, targetColorR, kLerpSpeed * kDeltaTime);
		objectR_->SetObjColor(currentColorR);

		///
		///	Lスティック押し込み時にも色を濃くする
		///

		static Vector4 currentColorL(1.0f, 1.0f, 1.0f, 1.0f);
		Vector4 targetColorL;
		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) {
			targetColorL = {0.3f, 0.3f, 0.3f, 1.0f};
		} else {
			targetColorL = {1.0f, 1.0f, 1.0f, 1.0f};
		}
		// 現在の色を線形補間で更新
		currentColorL = Lerp(currentColorL, targetColorL, kLerpSpeed * kDeltaTime);
		objectL_->SetObjColor(currentColorL);
	}
}
