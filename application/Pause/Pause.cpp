#include "Pause.h"
#include "SpriteCommon.h"
#include "application/Object/Player/Player.h"
#include "math/Easing.h"

void Pause::Init() {
    // インスタンス生成
    input_ = Input::GetInstance();
    backGround_ = std::make_unique<Sprite>();
    backGround_->Initialize("menu/backGround.png", spritePosition_, color_);

    // 各要素初期化
    alpha_E.start_.x = 0.0f;
    alpha_E.end_.x = 0.0f;
    alpha_E.T_ = 0.0f;
    EscapeCoolTime_ = 0.0f;

    // 各フラグ初期化
    isPause_ = false;
    CanEscape_ = false;
    previousIsPause_ = false;
    textMovedRight_ = false;
    prevEscapeState_ = false;
    prevStartState_ = false;

    backGamePos_ = {0.0f, 0.0f};
    backSelectPos_ = {0.0f, 0.0f};
    restartPos_ = {0.0f, 0.0f};
    stagePos_ = {0.0f, 0.0f};
    pointerPos_ = {0.0f, 0.0f};
    spritePosition_ = {0.0f, 0.0f};

    pointer_E.start_ = {-360.0f, 420.0f};
    pointer_E.end_ = {-360.0f, 420.0f};
    pointer_E.T_ = 0.0f;
    stage_E.start_.x = -360.0f;
    stage_E.end_.x = -360.0f;
    stage_E.T_ = 0.0f;
    single_E.start_.x = -360.0f;
    single_E.end_.x = -360.0f;
    single_E.T_ = 0.0f;
    two_E.start_.x = -360.0f;
    two_E.end_.x = -360.0f;
    two_E.T_ = 0.0f;
    backGame_E.start_.x = -360.0f;
    backGame_E.end_.x = -360.0f;
    backGame_E.T_ = 0.0f;
    backSelect_E.start_.x = -360.0f;
    backSelect_E.end_.x = -360.0f;
    backSelect_E.T_ = 0.0f;
    restart_E.start_.x = -360.0f;
    restart_E.end_.x = -360.0f;
    restart_E.T_ = 0.0f;

    EscapeCoolTime_ = 0.0f;
    pointerYT_ = 0.0f;
    transitionTimer_ = 0.0f;
    deltaTime_ = 1.0f / 60.0f;

    InitNumbers();
    InitText();
}

void Pause::Update() {
    // メニューを開く
    OpenMenu();

    // メニュー操作
    MenuOperation();

    // 文字の演出
    MoveText();

    // オブジェクト更新
    UpdateText();

    backGround_->SetSize({1280.0f, 720.0f});
    backGround_->SetAlpha(color_.w);
    // Debug();

    if (player_->GetSquareTransition() && isPause_) {
        isPause_ = false;
    }
}

void Pause::Draw(const ViewProjection &vp) {

    backGround_->Draw();
    Pointer_->Draw();
    Stage_->Draw();
    backGame_->Draw();
    backSelect_->Draw();
    Restart_->Draw();
    singleDigit_->Draw();
    twoDigit_->Draw();
}
#pragma region 文字関連

void Pause::InitText() {
    // インスタンス生成
    Stage_ = std::make_unique<Sprite>();
    backGame_ = std::make_unique<Sprite>();
    backSelect_ = std::make_unique<Sprite>();
    Restart_ = std::make_unique<Sprite>();
    Pointer_ = std::make_unique<Sprite>();

    // 初期化
    Stage_->Initialize("menu/stage.png", backGamePos_, {1, 1, 1, 1}, {0.5f, 0.5f});
    backGame_->Initialize("menu/backGame.png", backGamePos_, {1, 1, 1, 1}, {0.5f, 0.5f});
    backSelect_->Initialize("menu/backSelect.png", backSelectPos_, {1, 1, 1, 1}, {0.5f, 0.5f});
    Restart_->Initialize("menu/Restart.png", restartPos_, {1, 1, 1, 1}, {0.5f, 0.5f});
    Pointer_->Initialize("menu/pointer.png", pointerPos_, {1, 1, 1, 1}, {0.5f, 0.5f});

    // 初期位置(右から中央へやるため画面外)
    stagePos_ = {-360.0f, 100.0f};
    singlePos_ = {-360.0f, 103.0f};
    twoPos_ = {-360.0f, 103.0f};
    backGamePos_ = {-360.0f, 420.0f};
    backSelectPos_ = {-360.0f, 600.0f};
    restartPos_ = {-360.0f, 510.0f};
    pointerPos_ = {-360.0f, 420.0f};

    stageSize_ = Stage_->GetSize();
    singleSize_ = singleDigit_->GetSize();
    twoSize_ = twoDigit_->GetSize();
    backGameSize_ = backGame_->GetSize();
    backSelectSize_ = backSelect_->GetSize();
    restartSize_ = Restart_->GetSize();
    pointerSize_ = Pointer_->GetSize();
}

void Pause::UpdateText() {
    // 各種アニメーションパラメータを設定
    backGame_E.TMax_ = 0.5f;
    backSelect_E.TMax_ = 0.5f;
    restart_E.TMax_ = 0.5f;
    stage_E.TMax_ = 0.5f;
    pointer_E.TMax_ = 0.5f;
    single_E.TMax_ = 0.5f;
    two_E.TMax_ = 0.5f;

    // ポーズが解除された場合の初期設定
    if (!isPause_ && previousIsPause_) {
        pointer_E.start_.x = 500.0f;
        pointer_E.end_.x = 1640.0f;
        pointer_E.T_ = 0.0f;
        stage_E.start_.x = 600.0f;
        stage_E.end_.x = 1640.0f;
        stage_E.T_ = 0.0f;
        single_E.start_.x = 750.0f;
        single_E.end_.x = 1640.0f;
        single_E.T_ = 0.0f;
        two_E.start_.x = 710.0f;
        two_E.end_.x = 1640.0f;
        two_E.T_ = 0.0f;
        backGame_E.start_.x = 640.0f;
        backGame_E.end_.x = 1640.0f; // 右側に移動する終点位置
        backGame_E.T_ = 0.0f;
        backSelect_E.start_.x = 640.0f;
        backSelect_E.end_.x = 1640.0f;
        backSelect_E.T_ = 0.0f;
        restart_E.start_.x = 640.0f;
        restart_E.end_.x = 1640.0f;
        restart_E.T_ = 0.0f;
    }
    // ポーズがかかった場合の初期設定
    else if (isPause_ && !previousIsPause_) {
        pointer_E.start_.x = -360.0f;
        pointer_E.end_.x = 500.0f;
        pointer_E.T_ = 0.0f;
        stage_E.start_.x = -360.0f;
        stage_E.end_.x = 600.0f;
        stage_E.T_ = 0.0f;
        single_E.start_.x = -360.0f;
        single_E.end_.x = 750.0f;
        single_E.T_ = 0.0f;
        two_E.start_.x = -360.0f;
        two_E.end_.x = 710.0f;
        two_E.T_ = 0.0f;
        backGame_E.start_.x = -360.0f;
        backGame_E.end_.x = 640.0f;
        backGame_E.T_ = 0.0f;
        backSelect_E.start_.x = -360.0f;
        backSelect_E.end_.x = 640.0f;
        backSelect_E.T_ = 0.0f;
        restart_E.start_.x = -360.0f;
        restart_E.end_.x = 640.0f;
        restart_E.T_ = 0.0f;
    }

    // イージングによるアニメーション更新
    stage_E.T_ += deltaTime_;
    single_E.T_ += deltaTime_;
    two_E.T_ += deltaTime_;
    if (stage_E.T_ >= 0.1f) {
        pointer_E.T_ += deltaTime_;
        backGame_E.T_ += deltaTime_;
    }
    if (restart_E.T_ >= 0.1f) {
        backSelect_E.T_ += deltaTime_;
    }
    if (backGame_E.T_ >= 0.1f) {
        restart_E.T_ += deltaTime_;
    }

    // 時間の制限を適用
    if (pointer_E.T_ >= pointer_E.TMax_) {
        pointer_E.T_ = pointer_E.TMax_;
    }
    if (stage_E.T_ >= stage_E.TMax_) {
        stage_E.T_ = stage_E.TMax_;
    }
    if (single_E.T_ >= single_E.TMax_) {
        single_E.T_ = single_E.TMax_;
    }
    if (two_E.T_ >= two_E.TMax_) {
        two_E.T_ = two_E.TMax_;
    }
    if (backGame_E.T_ >= backGame_E.TMax_) {
        backGame_E.T_ = backGame_E.TMax_;
    }
    if (backSelect_E.T_ >= backSelect_E.TMax_) {
        backSelect_E.T_ = backSelect_E.TMax_;
    }
    if (restart_E.T_ >= restart_E.TMax_) {
        restart_E.T_ = restart_E.TMax_;
    }

    // イージング関数を使って位置を更新
    pointerPos_.x = EaseInOutBack<float>(pointer_E.start_.x, pointer_E.end_.x, pointer_E.T_, pointer_E.TMax_);
    stagePos_.x = EaseInOutBack<float>(stage_E.start_.x, stage_E.end_.x, stage_E.T_, stage_E.TMax_);
    singlePos_.x = EaseInOutBack<float>(single_E.start_.x, single_E.end_.x, single_E.T_, single_E.TMax_);
    twoPos_.x = EaseInOutBack<float>(two_E.start_.x, two_E.end_.x, two_E.T_, two_E.TMax_);
    backGamePos_.x = EaseInOutBack<float>(backGame_E.start_.x, backGame_E.end_.x, backGame_E.T_, backGame_E.TMax_);
    backSelectPos_.x = EaseInOutBack<float>(backSelect_E.start_.x, backSelect_E.end_.x, backSelect_E.T_, backSelect_E.TMax_);
    restartPos_.x = EaseInOutBack<float>(restart_E.start_.x, restart_E.end_.x, restart_E.T_, restart_E.TMax_);

    // すべてのテキストが左に移動しきったかをチェック
    if (backGamePos_.x >= 1640.0f && backSelectPos_.x >= 1640.0f && restartPos_.x >= 1640.0f) {
        textMovedRight_ = true; // フラグを設定
        stagePos_.x = -360.0f;
        singlePos_.x = -360.0f;
        twoPos_.x = -360.0f;
        backGamePos_.x = -360.0f;
        backSelectPos_.x = -360.0f;
        restartPos_.x = -360.0f;
        stage_E.start_.x = -360.0f;
        stage_E.end_.x = -360.0f;
        single_E.start_.x = -360.0f;
        single_E.end_.x = -360.0f;
        two_E.start_.x = -360.0f;
        two_E.end_.x = -360.0f;
        backGame_E.start_.x = -360.0f;
        backGame_E.end_.x = -360.0f;
        backSelect_E.start_.x = -360.0f;
        backSelect_E.end_.x = -360.0f;
        restart_E.start_.x = -360.0f;
        restart_E.end_.x = -360.0f;
    } else {
        textMovedRight_ = false; // フラグをリセット
    }

    // 現在のポーズ状態を前フレームの状態として保存
    previousIsPause_ = isPause_;
}

void Pause::MoveText() {
    Pointer_->SetPosition(pointerPos_);
    Stage_->SetPosition(stagePos_);
    backGame_->SetPosition(backGamePos_);
    backSelect_->SetPosition(backSelectPos_);
    Restart_->SetPosition(restartPos_);
    singleDigit_->SetPosition(singlePos_);
    twoDigit_->SetPosition(twoPos_);
    Stage_->SetSize(stageSize_ / 1.25f);
    backGame_->SetSize(backGameSize_ / 1.75f);
    backSelect_->SetSize(backSelectSize_ / 1.75f);
    Restart_->SetSize(restartSize_ / 1.75f);
    Pointer_->SetSize(pointerSize_ / 1.75f);
    singleDigit_->SetSize(singleSize_ / 1.5f);
    twoDigit_->SetSize(twoSize_ / 1.5f);
}
#pragma endregion

void Pause::OpenMenu() {
    alpha_E.TMax_ = 0.2f;

    // 前回のゲームパッドの状態を取得
    XINPUT_STATE joyState, prejoyState;
    input_->GetJoystickState(0, joyState);            // 現在のジョイスティック状態
    input_->GetJoystickStatePrevious(0, prejoyState); // 前回のジョイスティック状態

    // メニューを開く処理（EscまたはゲームパッドのSTARTボタン）
    if (!isPause_ && CanEscape_) {
        // キーボードEsc
        if (input_->TriggerKey(DIK_ESCAPE) && !prevEscapeState_) {
            alpha_E.start_.x = 0.0f;
            alpha_E.end_.x = 0.90f;
            alpha_E.T_ = 0.0f;
            isPause_ = true;
            EscapeCoolTime_ = 0.7f;  // クールダウンタイム開始
            prevEscapeState_ = true; // 前回押した状態を記録
        }
        // ゲームパッドのSTARTボタン
        else if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_START) && !(prejoyState.Gamepad.wButtons & XINPUT_GAMEPAD_START)) {
            alpha_E.start_.x = 0.0f;
            alpha_E.end_.x = 0.90f;
            alpha_E.T_ = 0.0f;
            isPause_ = true;
            EscapeCoolTime_ = 0.7f; // クールダウンタイム開始
        }
    }

    // クールタイム中はメニューが開けない
    if (EscapeCoolTime_ > 0.0f) {
        EscapeCoolTime_ -= deltaTime_;
        CanEscape_ = false;
    } else {
        CanEscape_ = true; // クールダウン終了後に再度開けるようにする
    }

    // メニューを閉じる処理（EscキーまたはゲームパッドのSTARTボタン）
    if (isPause_ && CanEscape_) {
        // キーボードEsc
        if (input_->TriggerKey(DIK_ESCAPE) && prevEscapeState_) {
            isPause_ = false;
            EscapeCoolTime_ = 0.7f;   // クールダウンタイム再開
            prevEscapeState_ = false; // 前回押した状態をリセット
        }
        // ゲームパッドのSTARTボタン
        else if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_START) && (prejoyState.Gamepad.wButtons & XINPUT_GAMEPAD_START)) {
            isPause_ = false;
            EscapeCoolTime_ = 0.7f; // クールダウンタイム再開
        }
    }

    // テキストが右に移動した場合の処理
    if (textMovedRight_) {
        alpha_E.start_.x = 0.90f;
        alpha_E.end_.x = 0.0f;
        alpha_E.T_ = 0.0f;
        textMovedRight_ = false;
        isFinish_ = true;
    }

    // イージング処理
    alpha_E.T_ += deltaTime_;
    if (alpha_E.T_ >= alpha_E.TMax_) {
        alpha_E.T_ = alpha_E.TMax_;
    }

    color_.w = EaseInSine<float>(alpha_E.start_.x, alpha_E.end_.x, alpha_E.T_, alpha_E.TMax_);
}

void Pause::MenuOperation() {
    const float easeTMax = 0.2f;              // ポインタのイージング時間
    const float pressInterval = 0.2f;         // 長押し時の入力インターバル
    static float pressTimerStickUp = 0.0f;    // スティック上方向の長押しタイマー
    static float pressTimerStickDown = 0.0f;  // スティック下方向の長押しタイマー
    static float pressTimerDpadUp = 0.0f;     // 十字キー上方向の長押しタイマー
    static float pressTimerDpadDown = 0.0f;   // 十字キー下方向の長押しタイマー
    static bool singlePressProcessed = false; // 単押しが処理済みかどうか

    // currentItem_ の範囲を制限
    if (currentItem_ < -2) {
        currentItem_ = 0;
    }
    if (currentItem_ > 0) {
        currentItem_ = -2;
    }

    if (isPause_) {
        isFinish_ = false;
        XINPUT_STATE joyState;
        if (input_->GetJoystickState(0, joyState)) {
            float stickY = static_cast<float>(joyState.Gamepad.sThumbLY) / 32767.0f; // 正規化 (-1.0 ～ 1.0)

            // スティック上方向
            if (stickY > 0.5f || (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)) {
                pressTimerStickUp += deltaTime_;
                pressTimerStickDown = 0.0f; // 下方向のタイマーリセット
                pressTimerDpadDown = 0.0f;

                if (pressTimerStickUp >= pressInterval || !singlePressProcessed) {
                    currentItem_++;
                    pointer_E.start_.y = pointerPos_.y;
                    pointerYT_ = 0.0f;
                    singlePressProcessed = true; // 単押しを処理済みにする
                    pressTimerStickUp = 0.0f;    // タイマーリセット
                }
            }
            // スティック下方向
            else if (stickY < -0.5f || (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)) {
                pressTimerStickDown += deltaTime_;
                pressTimerStickUp = 0.0f; // 上方向のタイマーリセット
                pressTimerDpadUp = 0.0f;

                if (pressTimerStickDown >= pressInterval || !singlePressProcessed) {
                    currentItem_--;
                    pointer_E.start_.y = pointerPos_.y;
                    pointerYT_ = 0.0f;
                    singlePressProcessed = true; // 単押しを処理済みにする
                    pressTimerStickDown = 0.0f;  // タイマーリセット
                }
            } else {
                // スティック入力がない場合、タイマーと単押しフラグをリセット
                pressTimerStickUp = 0.0f;
                pressTimerStickDown = 0.0f;
                singlePressProcessed = false;
            }
        }

        // キーボード操作
        if (input_->TriggerKey(DIK_W)) {
            currentItem_++;
            pointer_E.start_.y = pointerPos_.y;
            pointerYT_ = 0.0f;
        }
        if (input_->TriggerKey(DIK_S)) {
            currentItem_--;
            pointer_E.start_.y = pointerPos_.y;
            pointerYT_ = 0.0f;
        }
    } else {
        currentItem_ = 0;
    }

    // currentItem_ に応じたポインタ位置の設定
    if (currentItem_ == 0) {
        pointer_E.end_.y = 420.0f;
    } else if (currentItem_ == -1) {
        pointer_E.end_.y = 510.0f;
    } else if (currentItem_ == -2) {
        pointer_E.end_.y = 600.0f;
    }

    // イージングタイムの更新
    pointerYT_ += deltaTime_;
    if (pointerYT_ >= easeTMax) {
        pointerYT_ = easeTMax;
    }

    // 決定操作
    XINPUT_STATE joyState;
    if (isPause_) {
        if (currentItem_ == 0 && (input_->TriggerKey(DIK_SPACE))) {
            isPause_ = false;
            prevEscapeState_ = false;
        }
        if (currentItem_ == -1 && (input_->TriggerKey(DIK_SPACE))) {
            player_->SetTransitionStart();
            prevEscapeState_ = false;
        }
        if (input_->GetJoystickState(0, joyState)) {
            if (currentItem_ == 0 && (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) {
                isPause_ = false;
                prevEscapeState_ = false;
            }
            if (currentItem_ == -1 && (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) {
                player_->SetTransitionStart();
                prevEscapeState_ = false;
            }
        }
    }
    // ポインタ位置のイージング更新
    pointerPos_.y = EaseInSine<float>(pointer_E.start_.y, pointer_E.end_.y, pointerYT_, easeTMax);
}

void Pause::Debug() {
    ImGui::Begin("Pause");
    ImGui::Text("現在のアイテム %d", currentItem_);
    ImGui::Text("現在のステージ %d", stageNum_);
    ImGui::End();
}

void Pause::InitNumbers() {
    singleDigit_ = std::make_unique<Sprite>();
    twoDigit_ = std::make_unique<Sprite>();

    // 一桁目のモデルを設定
    int singleDigitValue = stageNum_ % 10;
    std::string singleDigitModelPath = "menu/" + std::to_string(singleDigitValue) + ".png";
    singleDigit_->Initialize(singleDigitModelPath, singlePos_, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});

    // 二桁目のモデルを設定
    int twoDigitValue = stageNum_ / 10;
    if (twoDigitValue > 0) {
        std::string twoDigitModelPath = "menu/" + std::to_string(twoDigitValue) + ".png";
        twoDigit_->Initialize(twoDigitModelPath, twoPos_, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
    } else {
        // 二桁目がない場合は0を表すモデルを設定
        twoDigit_->Initialize("menu/0.png", twoPos_, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
    }
}
