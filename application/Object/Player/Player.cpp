#define NOMINMAX
#include "Player.h"

// Engine
#include "Audio.h"
#include "ParticleEditor.h"
#include "math/Easing.h"
#include "math/myMath.h"
#include "myEngine/3d/line/DrawLine3D.h"
#include <myEngine/Frame/Frame.h>

void Player::Init(const std::string className, int currentStageNum) {
#ifdef _DEBUG
    dummyObject_ = std::make_unique<BaseObject>();
    dummyObject_->Init("dummy");
    dummyObject_->CreateModel("debug/Cube.obj");
    dummyObject_->SetTexture("debug/white.png");
#endif

    // ゴールガイドスプライト生成
    spriteGoalGuide_ = std::make_unique<Sprite>();
    spriteGoalGuide_->Initialize("game/goalGuide.png", {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
    spriteGoalGuide_->SetSize({160.0f, 90.0f});
    spriteGoalGuideTitle_ = std::make_unique<Sprite>();
    spriteGoalGuideTitle_->Initialize("game/startUi.png", {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
    spriteGoalGuideTitle_->SetSize({128.0f, 148.0f});

    input_ = Input::GetInstance();

    BaseObject::Init(className);
    BaseObject::SetRotationY(90.0f);
    BaseObject::CreateModel("animation/playerStandby.gltf");
    BaseObject::SetTexture("game/playerWhite.png"); // 白状態のプレイヤーテクスチャを設定
    BaseObject::CreateCollider();
    BaseObject::AddAnimation("animation/playerStandby.gltf");
    BaseObject::AddAnimation("animation/playerWalk.gltf");
    BaseObject::AddAnimation("animation/playerJump.gltf");
    BaseObject::AddAnimation("animation/playerGoal.gltf");
    Collider::SetVisible(false);

    ///
    ///	各パラメーター初期化
    ///

    gravityAcceleration_ = -0.01f; // 重力
    jumpAcceleration_ = 0.3f;      // ジャンプ初速

    /*xInvertRange_ = 3;
    yInvertRange_ = 3;*/

    goalGuideAlpha_ = 0.0f;

    ///
    ///	その他
    ///

    // SquareTransition初期化
    squareTransition_ = std::make_unique<SquareTransition>();
    squareTransition_->Initialize();

    // プレイヤー反転範囲スプライト生成
    spritePlayerArea_ = std::make_unique<Sprite>();
    spritePlayerArea_->Initialize("game/playerFlame.png", {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
    spritePlayerArea_->SetSize({165.0f, 165.0f});

    // Jsonからパラメーターの読み込み
    LoadFromJson();

    jumpSE_ = Audio::GetInstance()->LoadWave("player/playerJump.wav");
    landingSE_ = Audio::GetInstance()->LoadWave("player/playerLanding.wav");
    walkSE_ = Audio::GetInstance()->LoadWave("player/playerWalk.wav");
    gravitySE_ = Audio::GetInstance()->LoadWave("action/inversionGravity.wav");
    inversionSE_ = Audio::GetInstance()->LoadWave("action/inversion.wav");
    invertDisabledSE_ = Audio::GetInstance()->LoadWave("action/noInversion.wav");

    // 現在のステージ数をセット
    currentStageNum_ = currentStageNum;
    // 選択されたステージによって反転枠の大きさを変更する
    ChangeInvertRangeSizeForStageNum();
    coinGetSE_ = Audio::GetInstance()->LoadWave("action/getCoin.wav");

    coinEmitter_ = ParticleEditor::GetInstance()->GetEmitter("coin");

    runEmitter_ = ParticleEditor::GetInstance()->GetEmitter("smokerun");

    changeEmitter_ = ParticleEditor::GetInstance()->GetEmitter("ChangePlayer");

    moveCoolTime_ = 0.75f;
}

void Player::Update(MapChipField *mapChipField, bool title) {
#ifdef _DEBUG
    dummyObject_->SetScale({0.675f, 1.35f, 0.675f});
    dummyObject_->Update();
#endif

    // 各種瞬間判定フラグをリセット
    isCollectCoinOccurred_ = false; // なぜか下に記述するとずっとfalseになってしまうのでここに記述

    ///
    ///	毎フレーム更新処理
    ///

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

    InversMove();

    mapChipField_ = mapChipField;

    // コインの取得処理
    for (auto &coin : mapChipField_->GetCoins()) {
        // プレイヤーがコインに触れたかを判定
        if (IsCollidingCoin(*coin)) {
            coin->SetCollected(true); // コインを取得済みにする
        }
    }

    // プレイヤーの色状態をmapChipFieldに知らせる
    if (colorState_ == ColorState::White) {
        mapChipField_->SetIsPlayerWhite(true);
    } else if (colorState_ == ColorState::Black) {
        mapChipField_->SetIsPlayerWhite(false);
    }

    // 接地しているか天井に接触した際にはY方向速度をリセット
    if (collisionMapInfo_.hittingGround_) {
        velocity_.y = 0.0f;

    } else if (collisionMapInfo_.hittingCeiling_) {
        velocity_.y = 0.0f;
    }

    if (IsResetOccurred()) {
        moveCoolTime_ = 0.75f;
    }

    // 各種瞬間判定フラグをリセット
    isJumpOccurred_ = false;
    isBlockInversionOccurred_ = false;
    isResetOccurred_ = false;
    isGravityReversedOccurred_ = false;
    /*isCollectCoinOccurred_ = false;*/
    isInvertDisabled_ = false;

    ///
    ///	入力操作
    ///

    if (moveCoolTime_ < 0.0f) {
        HandleInput(title);
    } else {
        moveCoolTime_ -= 1.0f / 60.0f;
    }

    AnimaUpdate(title);

    RunParitcle();

    ///
    ///	重力を常に受ける
    ///

    if (!isInverting_) {                             // ブロック反転中には重力を加算しない
        if (!mapChipField_->IsAnyChipAnimating()) {  // アニメーション中のブロックが1つでもあれば重力を加算しない
            if (isGravityReversed_) {                // 重力反転中
                velocity_.y -= gravityAcceleration_; // 上向きに重力をかける (逆)
            } else {                                 // 通常重力
                velocity_.y += gravityAcceleration_; // 下向きに重力をかける（順）
            }
        }
    }

    ///
    ///	全てのブロックとの衝突判定とプレイヤーの押し戻し
    ///

    CheckCollisionAndResolve(title);

    ///
    ///	反転操作無効時には反転範囲のスプライトを揺らす
    ///

    // 反転操作が無効になった瞬間にシェイク開始
    if (isInvertDisabled_) {
        spriteShakeTimer_ = kShakeDuration; // シェイク時間のセット
    }

    ///
    ///	反転が成立した瞬間に拡大->縮小アニメーション開始
    ///
    if (isBlockInversionOccurred_) {
        spriteScaleTimer_ = kSpriteScaleDuration;
    }

    ///
    /// 色反転時にプレイヤー本体の拡縮を行う
    ///

    UpdateScalingAnimation();

#ifdef _DEBUG
    ImGui::Begin("player");

    if (ImGui::BeginTabBar(className_.c_str())) {
        if (ImGui::BeginTabItem("デバッグ")) {

            ImGui::DragFloat3("velocity", &velocity_.x);

            ImGui::Text("hittingGround : %d", collisionMapInfo_.hittingGround_);
            ImGui::Text("hittingCeiling : %d", collisionMapInfo_.hittingCeiling_);
            ImGui::Text("hittingLeft : %d", collisionMapInfo_.hittingLeft_);
            ImGui::Text("hittingRight : %d", collisionMapInfo_.hittingRight_);
            ImGui::Text("isOverlapping : %d", collisionMapInfo_.isOverlapping_);
            ImGui::Text("isTouchGoal : %d", isTouchGoal_);

            /*ImGui::Checkbox("ブロック反転中", &isInverting_);
            ImGui::Checkbox("重力反転中", &isGravityReversed_);*/

            /*ImGui::Text("TransitionStatus : %d", squareTransition_->GetCurrentStatus());
            ImGui::Text("TransitionIsFinished : %d", squareTransition_->IsFinished());*/

            bool flag[8] = {false};
            flag[0] = IsJumpOccurred();
            flag[1] = IsBlockInversionOccurred();
            flag[2] = IsResetOccurred();
            flag[3] = IsGravityReversedOccurred();
            flag[4] = IsLandedOccurred();
            flag[5] = IsCollectCoinOccurred();
            flag[6] = IsInvertDisabled();
            flag[7] = IsTouchGoalAndOnGround();

            ImGui::Checkbox("ジャンプした瞬間", &flag[0]);
            ImGui::Checkbox("ブロック反転した瞬間", &flag[1]);
            ImGui::Checkbox("リセットした瞬間", &flag[2]);
            ImGui::Checkbox("重力反転した瞬間", &flag[3]);
            ImGui::Checkbox("着地した瞬間", &flag[4]);
            ImGui::Checkbox("コインを取得した瞬間", &flag[5]);
            ImGui::Checkbox("反転操作が無効の瞬間", &flag[6]);
            ImGui::Checkbox("ゴールに触れていて地面にいる間", &flag[7]);

            if (colorState_ == ColorState::White) {
                ImGui::Text("現在の色 : 白");
            } else if (colorState_ == ColorState::Black) {
                ImGui::Text("現在の色 : 黒");
            }

            ImGui::Text("現在の取得コイン数 : %d", currentCoinCount_);

            ImGui::DragFloat3("最後に取得したコインの座標", &lastCollectedCoinPosition_.x);

            /*ImGui::Text("現在のステージ : %d", currentStageNum_);*/

            bool aiueoFlag = mapChipField_->IsAnyChipAnimating();
            ImGui::Checkbox("アニメーション中のブロックが1つでもあるかどうか", &aiueoFlag);

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
#endif
}

void Player::Draw(const ViewProjection &viewProjection, Vector3 offSet) {
#ifdef _DEBUG
    /*dummyObject_->Draw(viewProjection);*/
#endif

    BaseObject::Draw(viewProjection, {0.0f, offSetY_, 0.0f});

    // 反転可能範囲を描画
    /*DrawInvertArea();*/
}

void Player::DrawParticle(const ViewProjection &viewProjection) {
    CoinParticle();
    runEmitter_->Draw(viewProjection);
    coinEmitter_->Draw(viewProjection);
    changeEmitter_->Draw(viewProjection);
}

void Player::DrawSprite(const ViewProjection &viewProjection, bool title) {
    ///
    ///	プレイヤーの反転可能範囲画像について
    ///

    // プレイヤーのワールド座標をスクリーン座標に変換してspritePlayerAreaの位置をセット
    InvertAreaSpriteToPlayerPosition(viewProjection);
    // 現在の反転可能範囲の数値によってspritePlayerAreaのサイズを変更
    InvertAreaSpriteAdjust();

    // 反転無効時と反転成立時にspritePlayerAreaにアニメーションを適用する
    SpritePlayerAreaAnimation();

    // プレイヤー反転可能範囲の描画
    spritePlayerArea_->Draw();

    ///
    ///	ゴールガイド画像について
    ///

    // ゴールガイド画像をプレイヤーの位置に合わせる
    GoalGuideSpriteToPlayerPosition(viewProjection);

    // ゴールガイド画像の透明度を変更
    UpdateGoalGuideSpriteAlpha();

    // ゴールガイドの描画（alphaが0.0fよりも大きければ描画）
    if (title) {
        spriteGoalGuideTitle_->SetPosition({935.0f, 430.0f});
        spriteGoalGuideTitle_->SetAlpha(1.0f);
        spriteGoalGuideTitle_->Draw();
    } else {
        if (!isGravityReversed_) { // 重力反転時には描画されないようにする
            if (goalGuideAlpha_ > 0.0f) {
                spriteGoalGuide_->SetAlpha(goalGuideAlpha_);
                spriteGoalGuide_->Draw();
            }
        }
    }

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

bool Player::IsGoalReached(bool title) {
    /*   デバッグ用にO押したらクリアにする（あとで絶対消す）
      if (input_->TriggerKey(DIK_O)) {
          return true;
      }*/
    const float colliderYOffset = kHeight / 4.0f;
    // 現在位置の取得
    Vector3 position = this->transform_.translation_ + Vector3(0.0f, colliderYOffset, 0.0f);
    // プレイヤーの4つの角を計算
    Vector3 corners[4] = {
        {position.x - kWidth / 2, position.y + kHeight / 2, position.z}, // 左上
        {position.x + kWidth / 2, position.y + kHeight / 2, position.z}, // 右上
        {position.x - kWidth / 2, position.y - kHeight / 2, position.z}, // 左下
        {position.x + kWidth / 2, position.y - kHeight / 2, position.z}  // 右下
    };
    // ゴール位置の取得
    Vector3 goalPosition = mapChipField_->GetGoalPosition();
    float goalLeft = goalPosition.x - MapChipField::kChipSize;
    float goalRight = goalPosition.x + MapChipField::kChipSize;
    float goalTop = goalPosition.y + MapChipField::kChipSize;
    float goalBottom = goalPosition.y - MapChipField::kChipSize;
    // ループ外でフラグを初期化
    reached = false;
    // 各角がゴール内にあるかを判定
    for (const auto &corner : corners) {
        if (corner.x >= goalLeft && corner.x <= goalRight && corner.y >= goalBottom && corner.y <= goalTop) {
            reached = true; // ゴール内の角が見つかったらフラグを立てる
            break;          // 一つでも見つかれば、他の角のチェックは不要
        }
    }
    // ゴールに触れていて、なおかつ操作入力と接地状態があればゴール到達とする
    if (reached && CanJump()) {
        isTouchGoal_ = true; // ゴールに触れている状態をセット

        // コントローラーのAボタンのトリガー入力（単押し）を検出
        XINPUT_STATE joyState, joyStatePre;
        if (input_->GetJoystickState(0, joyState) && input_->GetJoystickStatePrevious(0, joyStatePre)) {
            // 前のフレームでAボタンが押されておらず、現在のフレームで押されている状態をチェック
            bool isCurrentPressed = (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
            bool isPreviousPressed = (joyStatePre.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;

            // 単押し判定（前のフレームで押されていなくて、現在押されている）
            if (isCurrentPressed && !isPreviousPressed && collisionMapInfo_.hittingGround_ && !isGravityReversed_) {
                SetClearAnima(title);
                return true;
            }
        }

        // キーボードのスペースキーのトリガー入力（単押し）
        if (input_->TriggerKey(DIK_SPACE) && collisionMapInfo_.hittingGround_ && !isGravityReversed_) { // 重力反転時はゴールできないようにする
            SetClearAnima(title);
            return true;
        }
    } else {
        isTouchGoal_ = false;
    }
    return false;
}

bool Player::GetGoalAnimaFinish() {
    static float localTimer = 0.0f; // ローカルタイマー（関数内で保持）

    if (isGoalAnimaFinish_) {
        localTimer += 1.0f / 60.0f; // 経過時間を加算

        // 1秒経過したらtrueを返す
        if (localTimer >= 1.0f) {
            return true;
        }
    } else {
        localTimer = 0.0f; // アニメーションが完了していない場合はリセット
    }

    return false;
}

bool Player::IsCollidingCoin(const Coin &coin) {
    const float colliderYOffset = 0.0f;

    // 現在位置の取得
    Vector3 position = this->transform_.translation_ + Vector3(0.0f, colliderYOffset, 0.0f);
    // プレイヤーの8つの点を計算
    Vector3 corners[8] = {
        {position.x - kWidth / 2, position.y + kHeight / 2, position.z}, // 左上
        {position.x + kWidth / 2, position.y + kHeight / 2, position.z}, // 右上
        {position.x - kWidth / 2, position.y - kHeight / 2, position.z}, // 左下
        {position.x + kWidth / 2, position.y - kHeight / 2, position.z}, // 右下
        {position.x - kWidth / 2, position.y, position.z},               // 中心左
        {position.x + kWidth / 2, position.y, position.z},               // 中心右
        {position.x, position.y + kHeight / 2, position.z},              // 中心上
        {position.x, position.y - kHeight / 2, position.z},              // 中心下
    };

    // コインとの当たり判定
    Vector3 coinPosition = coin.GetWorldPosition();

    float shrinkFactor = 0.7f; // コインの当たり判定のサイズを縮小（70％に）
    float halfSize = (MapChipField::kChipSize * shrinkFactor) / 2.0f;

    float coinLeft = coinPosition.x - halfSize;
    float coinRight = coinPosition.x + halfSize;
    float coinTop = coinPosition.y + halfSize;
    float coinBottom = coinPosition.y - halfSize;

    // プレイヤーの角がコインの範囲内にあるかをチェック
    for (const auto &corner : corners) {
        if (corner.x >= coinLeft && corner.x <= coinRight && corner.y >= coinBottom && corner.y <= coinTop) {
            currentCoinCount_++; // 現在のコイン数を1増やす

            lastCollectedCoinPosition_ = coinPosition; // 触れたコインの座標を記録しておく

            isCollectCoinOccurred_ = true; // コインを取得したことを記録（SE・エフェクト用）

            return true; // 4つの角のどれかが触れていたらtrue
        }
    }

    return false;
}

void Player::GoalGuideSpriteToPlayerPosition(const ViewProjection &viewProjection) {
    // spritePlayerAreaにプレイヤーのワールド座標を設定
    Vector3 playerWorldPosition = this->GetWorldPosition();

    // ビューポート行列を作成
    Matrix4x4 matViewport = MakeViewPortMatrix(0.0f, 0.0f, WinApp::kClientWidth, WinApp::kClientHeight, 0, 1);

    // ビュー行列とプロジェクション行列を合成
    Matrix4x4 matViewProjection = viewProjection.matView_ * viewProjection.matProjection_;
    Matrix4x4 matViewProjecitonViewport = matViewProjection * matViewport;

    // プレイヤーのワールド座標をスクリーン座標に変換
    Vector3 screenPosition = Transformation(playerWorldPosition, matViewProjecitonViewport);

    const float offsetY = 118.0f;

    spriteGoalGuide_->SetPosition({screenPosition.x, screenPosition.y - offsetY}); // プレイヤーの頭上に表示されるように変更
}

void Player::UpdateGoalGuideSpriteAlpha() {
    // ゴールに触れているかつ、接地状態であれば透明度を徐々に上げる
    if (isTouchGoal_ && collisionMapInfo_.hittingGround_) {
        goalGuideAlpha_ += alphaIncreaseSpeed;
        // そうでない場合には減少
    } else {
        goalGuideAlpha_ -= alphaDecreaseSpeed;
    }

    // 透明度を0.0f ~ 1.0fの範囲に制限
    goalGuideAlpha_ = std::clamp(goalGuideAlpha_, 0.0f, 1.0f);

    spriteGoalGuide_->SetAlpha(goalGuideAlpha_);
}

void Player::UpdateScalingAnimation() {
    if (isScaling_) {
        // 経過時間の更新
        scaleTimer_ += kDeltaTime;

        // 指定秒を超えたら終了
        if (scaleTimer_ >= kScaleDuration) {
            scaleTimer_ = kScaleDuration;
            if (colorState_ == ColorState::Black) {
                changeEmitter_->SetTexture("particle/ChangeBlack.png");
                changeEmitter_->SetPosition({GetCenterPosition().x, GetCenterPosition().y, GetCenterPosition().z});
                changeEmitter_->UpdateOnce();
            }
            if (colorState_ == ColorState::White) {
                changeEmitter_->SetTexture("particle/ChangeWhite.png");
                changeEmitter_->SetPosition({GetCenterPosition().x, GetCenterPosition().y, GetCenterPosition().z});
                changeEmitter_->UpdateOnce();
            }
            isScaling_ = false;
        }

        float halfTime = kScaleDuration / 2.0f;
        float newScaleValue = initialScale_;

        float targetRotY;
        if (prevRotY_ > 0.0f) {
            targetRotY = 720.0f + 90.0f; // 右向き
        } else {
            targetRotY = -720.0f - 90.0f; // 左向き
        }

        // 回転の補間
        float newRotationValue = EaseOutQuad(0.0f, targetRotY, scaleTimer_, kScaleDuration);

        if (scaleTimer_ <= halfTime) {
            newScaleValue = EaseInQuad(initialScale_, targetScale_, scaleTimer_, halfTime);
        } else {
            if (!isChangedColor_) {

                isChangedColor_ = true;
            }

            float t = scaleTimer_ - halfTime;
            newScaleValue = EaseInQuad(targetScale_, initialScale_, t, halfTime);
        }
        this->SetScale({newScaleValue, newScaleValue, newScaleValue});

        // 回転の適用
        this->SetRotation({GetCenterRotation().x, degreesToRadians(newRotationValue), 0.0f});
    }
}

void Player::HandleInput(bool title) {
#pragma region ゲームパッド入力
    // 前フレームの押下状態を保存
    static bool wasPressedA = false;  // Aボタン
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

            if (!mapChipField_->IsAnyChipAnimating()) { // ブロックが1つでもアニメーションしていたら左右移動できないように
                if (abs(leftStickX) > deadZone) {
                    const float maxStickValue = 32767.0f;
                    float moveX = (abs(leftStickX) > deadZone) ? (leftStickX / maxStickValue) * kMoveSpeed : 0.0f;

                    // 移動量を反映
                    velocity_.x = moveX;
                }
            }
        }

        ///
        ///	ジャンプ入力
        ///

        bool isPressedA = joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A;

        // Aボタンが押された瞬間のみ
        if (isPressedA && !wasPressedA && CanJump() && !IsGoalReached(title)) {
            if (isGravityReversed_) { // 重力反転中
                // 天井にいる場合のみ
                if (collisionMapInfo_.hittingCeiling_) {
                    velocity_.y = -jumpAcceleration_; // 下向き (逆)

                    // ジャンプしたことを記録（SE・エフェクト用）
                    isJumpOccurred_ = true;
                    isJump_ = true;
                }
            } else {
                // 地面にいる場合のみ
                if (collisionMapInfo_.hittingGround_) {
                    velocity_.y = jumpAcceleration_; // 上向き (順)

                    // ジャンプしたことを記録（SE・エフェクト用）
                    isJumpOccurred_ = true;
                    isJump_ = true;
                }
            }
        }

        // 前フレームの状態を記録
        wasPressedA = isPressedA;

        ///
        ///	範囲内のブロック反転入力
        ///

        bool isPressedRB = joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

        /*if (isPressedRB && !wasPressedRB && blockInvertCooldown_ > 0) {
                isInvertDisabled_ = true;
        }*/

        // RBボタンが押された瞬間のみ
        if (isPressedRB && !wasPressedRB && blockInvertCooldown_ <= 0.0f) { // クールタイム中には反転できない
            if (!isInverting_ && !collisionMapInfo_.isOverlapping_) {       // ブロック反転中には反転できない && ブロックに埋まっていたら反転できない
                if (!mapChipField_->IsAnyChipAnimating()) {                 // ブロックが1つでもアニメーション中なら反転できないように
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
                            // 反転が成立したら強制的にシェイクを終わらせる
                            spriteShakeTimer_ = 0.0f;

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
                                isScaling_ = true;
                                scaleTimer_ = 0.0f;

                                prevRotY_ = this->GetCenterRotation().y;

                                // 現在が白の場合、テクスチャと色状態を黒に変更
                                if (colorState_ == ColorState::White) {
                                    this->SetTexture("game/player.png");
                                    colorState_ = ColorState::Black;

                                    // 現在が黒の場合、テクスチャと色状態を白に変更
                                } else if (colorState_ == ColorState::Black) {
                                    this->SetTexture("game/playerWhite.png");
                                    colorState_ = ColorState::White;
                                }
                            }

                            // ブロック反転したことを記録（SE・エフェクト用）
                            isBlockInversionOccurred_ = true;

                            // 反転範囲内に反転可能ブロックが無かった場合
                        } else {
                            isInvertDisabled_ = true; // 反転無効であることを知らせる
                        }
                    }
                }
            }
        }

        // RBを押した際にブロックに埋まっていた場合にも反転無効にする
        if (isPressedRB && !wasPressedRB && blockInvertCooldown_ <= 0.0f) {
            if (collisionMapInfo_.isOverlapping_) {
                blockInvertCooldown_ = kBlockInvertCooldownTime; // ブロック反転クールタイムを設定

                isInvertDisabled_ = true; // 反転無効であることを知らせる
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
    if (!isInverting_) {                            // ブロック反転中には移動できない
        if (!mapChipField_->IsAnyChipAnimating()) { // ブロックが1つでもアニメーションしていたら左右移動できないように
            if (input_->PushKey(DIK_A)) {
                velocity_.x = -kMoveSpeed;
            }
            if (input_->PushKey(DIK_D)) {
                velocity_.x = kMoveSpeed;
            }
        }
    }

    ///
    ///	ジャンプ入力
    ///

    if (input_->TriggerKey(DIK_W) && CanJump()) {
        if (isGravityReversed_) { // 重力反転中
            // 天井にいる場合のみ
            if (collisionMapInfo_.hittingCeiling_) {
                velocity_.y = -jumpAcceleration_; // 下向き (逆)
                // ジャンプしたことを記録（SE・エフェクト用）
                isJumpOccurred_ = true;
                isJump_ = true;
            }
        } else {
            // 地面にいる場合のみ
            if (collisionMapInfo_.hittingGround_) {
                velocity_.y = jumpAcceleration_; // 上向き (順)

                // ジャンプしたことを記録（SE・エフェクト用）
                isJumpOccurred_ = true;
                isJump_ = true;
            }
        }
    }

    ///
    ///	範囲内のブロック反転入力
    ///

    if (input_->TriggerKey(DIK_SPACE) && blockInvertCooldown_ <= 0.0f) { // クールタイム中には反転できない
        if (!isInverting_ && !collisionMapInfo_.isOverlapping_) {        // ブロック反転中には反転できない && ブロックに埋まっていたら反転できない
            if (!mapChipField_->IsAnyChipAnimating()) {                  // ブロックが1つでもアニメーション中なら反転できないように
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
                        // 反転が成立したら強制的にシェイクを終わらせる
                        spriteShakeTimer_ = 0.0f;

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
                            isScaling_ = true;
                            scaleTimer_ = 0.0f;

                            prevRotY_ = this->GetCenterRotation().y;

                            // 現在が白の場合、テクスチャと色状態を黒に変更
                            if (colorState_ == ColorState::White) {
                                this->SetTexture("game/player.png");
                                colorState_ = ColorState::Black;
                                // 現在が黒の場合、テクスチャと色状態を白に変更
                            } else if (colorState_ == ColorState::Black) {
                                this->SetTexture("game/playerWhite.png");
                                colorState_ = ColorState::White;
                            }
                        }

                        // ブロック反転したことを記録（SE・エフェクト用）
                        isBlockInversionOccurred_ = true;

                        // 反転範囲内に反転可能ブロックが無かった場合
                    } else {
                        isInvertDisabled_ = true; // 反転無効であることを知らせる
                    }
                }
            }
        }
    }

    // SPACEを押した際にブロックに埋まっていた場合にも反転無効にする
    if (input_->TriggerKey(DIK_SPACE) && blockInvertCooldown_ <= 0.0f) {
        if (collisionMapInfo_.isOverlapping_) {
            blockInvertCooldown_ = kBlockInvertCooldownTime; // ブロック反転クールタイムを設定

            isInvertDisabled_ = true; // 反転無効であることを知らせる
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
                isLanded_ = true;
                return true;
            }

            prevHittingGround_ = currentHittingGround;

            // 重力が逆の場合
        } else {
            bool currentHittingGround = collisionMapInfo_.hittingCeiling_;

            // 着地した瞬間のみを判定
            if (!prevHittingGround_ && currentHittingGround) {
                prevHittingGround_ = currentHittingGround;
                isLanded_ = true;
                return true;
            }

            prevHittingGround_ = currentHittingGround;
        }
    }
    isLanded_ = false;
    return false;
}

void Player::SpritePlayerAreaAnimation() {
    // 反転無効時のシェイク処理
    if (spriteShakeTimer_ > 0.0f) {
        float shakeStrength = 10.0f; // シェイクの強さ

        // 減衰係数
        float damping = spriteShakeTimer_ / kShakeDuration;

        float shakeOffset = shakeStrength * damping * std::sinf(spriteShakeTimer_ * 60.0f); // 振動の速さを調整

        Vector2 currentPos = spritePlayerArea_->GetPosition();
        spritePlayerArea_->SetPosition({currentPos.x + shakeOffset, currentPos.y});

        // シェイク時間の減少
        spriteShakeTimer_ -= kDeltaTime;
    }
    // 反転成立時の拡縮処理
    if (spriteScaleTimer_ > 0.0f) {
        constexpr float minScale = 1.0f;
        constexpr float maxScale = 1.2f;
        float progress = (kSpriteScaleDuration - spriteScaleTimer_) / kSpriteScaleDuration; // 0 -> 1 へ進行

        // 拡大（前半 0.0f ~ 0.5f）
        float scaleFactor;
        if (progress < 0.5f) {
            scaleFactor = EaseOutQuad(minScale, maxScale, progress, 0.5f);
            // 縮小（後半 0.5f ~ 1.0f）
        } else {
            scaleFactor = EaseOutQuad(maxScale, minScale, progress - 0.5f, 0.5f);
        }

        // スプライトのスケールを設定
        spritePlayerArea_->SetSize({spritePlayerArea_->GetSize().x * scaleFactor, spritePlayerArea_->GetSize().y * scaleFactor});

        // タイマーを減少
        spriteScaleTimer_ -= kDeltaTime;
    }
}

bool Player::IsWalking() {
    if (input_->PushKey(DIK_D) || input_->PushKey(DIK_A)) {
        isWalking_ = true;
    } else {
        isWalking_ = false;
    }

    // PAD入力の判定を追加
    XINPUT_STATE joyState;
    if (input_->GetJoystickState(0, joyState)) {
        float leftStickX = joyState.Gamepad.sThumbLX;
        const float kDeadZone = 4000.0f; // デッドゾーンの設定

        if (abs(leftStickX) > kDeadZone) {
            isWalking_ = true;
        }
    } else {
        isWalking_ = false;
    }

    if (isWalking_ && collisionMapInfo_.hittingGround_) {
        if (walkSEcoolTime_ < 0) {
            walkSEcoolTime_ = 0.4f;
            return true;
        }
        walkSEcoolTime_ -= Frame::DeltaTime();
    }
    return false;
}

void Player::Reset(bool title) {
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
        this->velocity_ = {0.0f, 0.0f, 0.0f};
        // プレイヤーの重力状態をリセット
        isGravityReversed_ = false;
        // プレイヤーの色状態をリセット（とりあえずデフォルトを白としておく）
        this->SetTexture("game/playerWhite.png");
        colorState_ = ColorState::White;
        // 現在の取得コイン数をリセット
        currentCoinCount_ = 0;
        // スケールをリセット
        this->SetScale(kDefaultScale);

        // マップのリセット
        mapChipField_->ResetMapChip();
        if (title) {
            mapChipField_->SetGoalModel();
        }
        // マップの所持する重力状態をリセット
        mapChipField_->SetIsGravityReversed(false);
        // マップの所持するプレイヤーの色状態をリセット
        mapChipField_->SetIsPlayerWhite(true); // 最初は白から始まるため

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

void Player::PlaySE() {
    Audio *audio = Audio::GetInstance();
    if (IsJumpOccurred()) {
        audio->PlayWave(jumpSE_, 0.05f);
    }
    if (IsLandedOccurred()) {
        audio->PlayWave(landingSE_, 0.05f);
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
    if (IsInvertDisabled()) {
        audio->PlayWave(invertDisabledSE_, 0.1f);
    }
}

void Player::BaseUpdate() {

    isGoalAnimaFinish_ = BaseObject::AnimaIsFinish();
    BaseObject::Update();
}

void Player::AnimaUpdate(bool title) {
    if (!IsGoalReached(title)) {
        if (velocity_.y == 0 && !isJump_) {
            if (velocity_.x == 0) {
                BaseObject::SetLoop(true);
                BaseObject::SetAnima("animation/playerStandby.gltf");
            } else {
                BaseObject::SetLoop(true);
                BaseObject::SetAnima("animation/playerWalk.gltf");
            }
        } /* else {
                 BaseObject::SetLoop(false);
                 BaseObject::SetAnima("animation/playerJump.gltf");
         }*/
        if (!isGravityReversed_) {
            if (velocity_.x > 0) {
                BaseObject::SetRotationY(degreesToRadians(90.0f));
            }
            if (velocity_.x < 0) {
                BaseObject::SetRotationY(degreesToRadians(-90.0f));
            }
        } else {
            if (velocity_.x > 0) {
                BaseObject::SetRotationY(degreesToRadians(-90.0f));
            }
            if (velocity_.x < 0) {
                BaseObject::SetRotationY(degreesToRadians(90.0f));
            }
        }
    } else {
        /*  if (!title) {
              BaseObject::SetLoop(false);
              BaseObject::SetAnima("animation/playerGoal.gltf");
              BaseObject::SetRotationY(degreesToRadians(90.0f));
          }*/
    }

    if (isJump_) {
        BaseObject::SetLoop(false);
        BaseObject::SetAnima("animation/playerJump.gltf");

        if (collisionMapInfo_.hittingCeiling_ || collisionMapInfo_.hittingGround_) {
            jumpCooltime += 0.1f;
        } else {
            jumpCooltime = 0.0f;
        }

        if (jumpCooltime > 0.1f) {
            isJump_ = false;
        }
    } else {
        jumpCooltime = 0.0f;
    }

    /*   if (input_->TriggerKey(DIK_W)) {
           BaseObject::SetLoop(false);
           BaseObject::SetAnima("animation/playerJump.gltf");
       }
       XINPUT_STATE joyState;
       if (input_->GetJoystickState(0, joyState)) {
           if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
               BaseObject::SetLoop(false);
               BaseObject::SetAnima("animation/playerJump.gltf");
           }
       }*/
}

void Player::InversMove() {
    float startPos = 0.0f;
    float endPos = 0.0f;
    float endRote = 0.0f;
    const float EaseTMax = 0.5f;

    if (!isGravityReversed_) {
        startPos = 1.5f;
        endPos = 0.0f;
        endRote = degreesToRadians(0.0f);
    } else {
        startPos = 0.0f;
        endPos = 1.5f;
        endRote = degreesToRadians(180.0f);
    }

    // 重力反転が発生したら、開始位置と開始角度を保存する
    if (IsGravityReversedOccurred()) {
        inversT_ = 0.0f;
        startRote_ = BaseObject::GetCenterRotation().x; // 現在の角度を記録
    }

    if (inversT_ < EaseTMax) {
        inversT_ += 1.0f / 60.0f;
    } else {
        inversT_ = EaseTMax;
    }

    offSetY_ = EaseInSine<float>(startPos, endPos, inversT_, EaseTMax);
    BaseObject::SetRotationX(EaseInSine<float>(startRote_, endRote, inversT_, EaseTMax));
}

void Player::CoinParticle() {

    if (IsCollectCoinOccurred()) {
        coinEmitter_->SetPosition({GetLastCollectedCoinPosition().x, GetLastCollectedCoinPosition().y, GetLastCollectedCoinPosition().z});
        coinEmitter_->UpdateOnce();
        Audio::GetInstance()->PlayWave(coinGetSE_, 0.1f);
    }
}

void Player::RunParitcle() {
    if (isWalking_ && collisionMapInfo_.hittingGround_) {
        runEmitter_->SetPosition({GetCenterPosition().x, GetCenterPosition().y - 0.3f, GetCenterPosition().z});
        // runEmitter_->SetPositionY(GetCenterPosition().y - 0.8f);
        if (velocity_.x > 0) {
            runEmitter_->SetRotateY(degreesToRadians(0.0f));
        }
        if (velocity_.x < 0) {
            runEmitter_->SetRotateY(degreesToRadians(180.0f));
        }
        runEmitter_->Update();
    }
}

void Player::CheckCollisionAndResolve(bool title) {
    /// X移動
    BaseObject::transform_.translation_.x += velocity_.x;

    /// 衝突判定
    CollisionMapInfo collisionMapInfoX = GetMapCollisionInfo(title);

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
    if (!isInverting_) {                            // ブロック反転中には移動しない
        if (!mapChipField_->IsAnyChipAnimating()) { // ブロックがどれか1つでもアニメーションしていたら移動しない
            BaseObject::transform_.translation_.y += velocity_.y;
        }
    }

    /// 衝突判定
    CollisionMapInfo collisionMapInfoY = GetMapCollisionInfo(title);

    const float colliderYOffset = kHeight / 4.0f;

    /// 押し戻し
    if (collisionMapInfoY.hittingGround_) {
        Vector3 blockPosition = collisionMapInfoY.blockY->GetWorldPosition();
        float blockBottom = blockPosition.y + MapChipField::kChipSize / 2;
        BaseObject::transform_.translation_.y = blockBottom + kHeight / 2 - colliderYOffset + kBlank; // 地面の位置に押し戻し
    } else if (collisionMapInfoY.hittingCeiling_) {
        Vector3 blockPosition = collisionMapInfoY.blockY->GetWorldPosition();
        float blockTop = blockPosition.y - MapChipField::kChipSize / 2;
        BaseObject::transform_.translation_.y = blockTop - kHeight / 2 - kBlank - colliderYOffset; // 天井の位置に押し戻し
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
        {minX, maxY, maxZ} // 上面
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
        {3, 7} // 側面
    };

    // エッジを描画
    for (const auto &edge : edges) {
        DrawLine3D::GetInstance()->SetPoints(vertices[edge.first], vertices[edge.second], {1.0f, 1.0f, 1.0f, 1.0f});
    }
}

void Player::InvertAreaSpriteToPlayerPosition(const ViewProjection &viewProjection) {
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

void Player::InvertAreaSpriteAdjust() {
    Vector2 spriteSize;

    // 反転可能範囲の数値によってスプライトのサイズを設定する
    const float sizes[] = {0.0f, 55.0f, 0.0f, 165.0f, 0.0f, 275.0f, 0.0f, 383.0f, 0.0f, 490.0f, 0.0f, 598.0f}; // 目視で合わせた各サイズ

    // xサイズ変更
    if (xInvertRange_ >= 1 && xInvertRange_ <= 11 && xInvertRange_ % 2 == 1) {
        spriteSize.x = sizes[xInvertRange_];
    }

    // yサイズ変更
    if (yInvertRange_ >= 1 && yInvertRange_ <= 11 && yInvertRange_ % 2 == 1) {
        spriteSize.y = sizes[yInvertRange_];
    }

    spritePlayerArea_->SetSize(spriteSize);
}

void Player::ChangeInvertRangeSizeForStageNum() {
    switch (currentStageNum_) {
    case -1:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 1:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 2:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 3:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 4:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 5:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 6:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 7:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 8:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 9:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 10:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 11:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 12:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 13:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 14:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    case 15:
        xInvertRange_ = 3;
        yInvertRange_ = 3;
        break;
    default:
        break;
    }
}

Player::CollisionMapInfo Player::GetMapCollisionInfo(bool title) {
    CollisionMapInfo info;

    // ずらす分（新しい高さと元の高さの差の半分）
    const float colliderYOffset = kHeight / 4.0f;
    // 衝突判定用の実際の中心は、transform_.translation_ から上方向にオフセット
    Vector3 position = this->transform_.translation_ + Vector3(0.0f, colliderYOffset, 0.0f);

#ifdef _DEBUG
    dummyObject_->SetWorldPosition(position);
#endif

    // 重なり判定のオフセット（プレイヤーの実際のサイズよりも少し減らした値で判定）
    const float overlapOffsetX = (kWidth / 2) - 0.02f;
    const float overlapOffsetY = (kHeight / 2) - 0.02f;

    // 重なり判定用の6点
    Vector3 checkPoints[6] = {
        {position.x - overlapOffsetX, position.y + overlapOffsetY, position.z}, // 左上
        {position.x + overlapOffsetX, position.y + overlapOffsetY, position.z}, // 右上
        {position.x - overlapOffsetX, position.y - overlapOffsetY, position.z}, // 左下
        {position.x + overlapOffsetX, position.y - overlapOffsetY, position.z}, // 右下
        {position.x - overlapOffsetX, position.y, position.z},                  // 中心左
        {position.x + overlapOffsetX, position.y, position.z}                   // 中心右
    };

    // プレイヤーの4つの角を計算
    Vector3 corners[4] = {
        {position.x - kWidth / 2, position.y + kHeight / 2, position.z}, // 左上
        {position.x + kWidth / 2, position.y + kHeight / 2, position.z}, // 右上
        {position.x - kWidth / 2, position.y - kHeight / 2, position.z}, // 左下
        {position.x + kWidth / 2, position.y - kHeight / 2, position.z}  // 右下
    };

    // 中心左と中心右の点を計算
    Vector3 centerLeft = {position.x - kWidth / 2, position.y, position.z};  // 中心左
    Vector3 centerRight = {position.x + kWidth / 2, position.y, position.z}; // 中心右

    // 全てのブロックを取得
    const auto blocks = mapChipField_->GetBlocks();
    const float blockSize = MapChipField::kChipSize;

    // 全てのブロックとの衝突判定
    for (const auto &block : blocks) {
        // ブロックの位置と範囲を計算
        Vector3 blockPosition = block->GetWorldPosition();

        // 空ブロックとは判定をとらない
        if (block->type_ == Block::ChipType::Empty) {
            continue;
        }

        float blockLeft = blockPosition.x - blockSize / 2;
        float blockRight = blockPosition.x + blockSize / 2;
        float blockTop = blockPosition.y + blockSize / 2;
        float blockBottom = blockPosition.y - blockSize / 2;

        // titleフラグがtrueで、Goalの場合の上方向判定の増加
        if (title && block->type_ == Block::ChipType::Goal) {
            blockTop += blockSize; // 上方向に1ブロック分増やす
        }

        // 重なり判定（プレイヤーの中心+-オフセットがブロックに接触しているか）
        for (int i = 0; i < 6; ++i) {
            if (checkPoints[i].x >= blockLeft && checkPoints[i].x <= blockRight && checkPoints[i].y >= blockBottom && checkPoints[i].y < blockTop) {
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
                } else if (i >= 2) {     // 左下・右下
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
    j["gravityAcceleration"] = {gravityAcceleration_};
    j["jumpAcceleration"] = {jumpAcceleration_};

    /*j["xInvertRange"] = {xInvertRange_};
    j["yInvertRange"] = {yInvertRange_};*/

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

    /*if (j.contains("xInvertRange") && j["xInvertRange"].is_array()) {
        xInvertRange_ = j["xInvertRange"][0];
    }
    if (j.contains("yInvertRange") && j["yInvertRange"].is_array()) {
        yInvertRange_ = j["yInvertRange"][0];
    }*/
}
