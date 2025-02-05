#include "UIObject.h"

// C++
#include <cmath>

void UIObject::Init(int currentStageNum) {
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

    filter_ = std::make_unique<BaseObject>();
    filter_->Init("filter");
    filter_->CreateModel("debug/plane.obj");
    filter_->SetTexture("debug/filter2.png");

    filterColor_ = {1.0f, 1.0f, 1.0f, 0.4f};

    // 各ボタンオブジェクトを、栞オブジェクトと親子付け
    objectL_->SetParent(&objectUI_->GetWorldTransform());
    objectA_->SetParent(&objectUI_->GetWorldTransform());
    objectR_->SetParent(&objectUI_->GetWorldTransform());

	///
	///	スプライト生成
	/// 
	
	spritePause_ = std::make_unique<Sprite>();
	spritePause_->Initialize("game/pause.png", {64.0f, 64.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
	spritePause_->SetSize({96.0f, 96.0f});

    // 現在のステージ数を格納
    currentStageNum_ = currentStageNum;
	// 現在のステージによってUIの位置を調整する
    AdjustUIPositionForStageNum();

    // 位置調整
    objectL_->SetRotation({degreesToRadians(-5.2f), 0.0f, 0.0f});

    objectR_->SetRotation({degreesToRadians(7.2f), 0.0f, 0.0f});
    objectR_->SetWorldPosition({0.0f, -0.2f, -0.1f});
}

void UIObject::Update() {
    // パッド入力による反応
    InputReaction();
    // オブジェクト更新
    objectBook_->Update();
    objectUI_->Update();
    objectL_->Update();
    objectA_->Update();
    objectR_->Update();
    filter_->SetObjColor(filterColor_);
    filter_->Update();
}

void UIObject::Draw(const ViewProjection &viewProjection) {
    objectBook_->Draw(viewProjection);
    objectUI_->Draw(viewProjection);
    objectL_->Draw(viewProjection);
    objectA_->Draw(viewProjection);
    objectR_->Draw(viewProjection);
    filter_->Draw(viewProjection);
}

void UIObject::DrawSprite() { spritePause_->Draw(true); }

void UIObject::DebugImGui()
{
	objectBook_->DebugImGui();
	objectUI_->DebugImGui();
	/*objectL_->DebugImGui();
	objectA_->DebugImGui();
	objectR_->DebugImGui();*/

	ImGui::Begin("UIObject.param");
    ImGui::Text("現在のステージ番号 : %d", currentStageNum_);
	ImGui::End();
}

void UIObject::InputReaction() {
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

        ///
        ///	ポーズボタンの押下時にも色を濃くする
        ///

        static Vector4 currentColorPause(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 targetColorPause;
        if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_START) {
            targetColorPause = {0.3f, 0.3f, 0.3f, 1.0f};
        } else {
            targetColorPause = {1.0f, 1.0f, 1.0f, 1.0f};
        }
        // 現在の色を線形補間で更新
        currentColorPause = Lerp(currentColorPause, targetColorPause, kLerpSpeed * kDeltaTime);
        spritePause_->SetColor({currentColorPause.x, currentColorPause.y, currentColorPause.z});
        spritePause_->SetAlpha(currentColorPause.w);
    }
}

void UIObject::AdjustUIPositionForStageNum() {
    switch (currentStageNum_) {
    case -1:
        /*これがデフォルト値になる*/

        ///
        /// 栞オブジェクトのトランスフォーム
        /// 
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
		/// 本オブジェクトのトランスフォーム
        /// 
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});

		break;
    case 1:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;
    case 2:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-6.8f, -14.5f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({13.7f, -9.830f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.9f, 4.9f, 4.9f});
        break;
    case 3:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-6.6f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({16.49f, -5.53f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({5.0f, 4.5f, 4.5f});
        break;
    case 4:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-5.3f, -12.4f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({16.19f, -7.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({5.0f, 4.5f, 4.6f});
        break;
    case 5:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -14.5f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -9.63f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;
    case 6:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-8.5f, -16.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({13.09f, -10.83f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.7f, 4.5f, 4.5f});
        break;
    case 7:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-4.2f, -16.9f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({16.39f, -11.83f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({5.1f, 4.5f, 4.5f});
        break;
    case 8:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-8.5f, -17.2f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({14.99f, -13.13f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.6f, 4.6f, 4.6f});
        break;
    case 9:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-5.3f, -15.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.79f, -10.13f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.9f, 4.8f, 4.8f});
        break;
    case 10:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;
    case 11:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;
    case 12:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;
    case 13:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;
    case 14:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;
    case 15:
        ///
        /// 栞オブジェクトのトランスフォーム
        ///
        objectUI_->SetWorldPosition({-9.0f, -10.0f, 0.0f});
        objectUI_->SetRotation({degreesToRadians(-86.699f), degreesToRadians(-27.5f), 0.314f});
        objectUI_->SetScale({2.5f, 2.5f, 2.5f});

        ///
        /// 本オブジェクトのトランスフォーム
        ///
        objectBook_->SetWorldPosition({12.69f, -5.33f, 7.87f});
        objectBook_->SetRotation({-1.57f, 0.0f, 0.0f});
        objectBook_->SetScale({4.5f, 4.5f, 4.5f});
        break;

    default:
        break;
    }
}
