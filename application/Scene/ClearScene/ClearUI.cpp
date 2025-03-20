#include "ClearUI.h"
#include "Audio.h"
#include "Easing.h"
#include "Input.h"
#include <ParticleEditor.h>

void ClearUI::Init(bool isPlayerWhite) {
    book_ = std::make_unique<BaseObject>();
    book_->Init("book");
    book_->CreateModel("game/openBook.obj");

    stage_ = std::make_unique<BaseObject>();
    stage_->Init("clear_stage");
    stage_->CreateModel("clear/stage.obj");

    nextStage_ = std::make_unique<BaseObject>();
    nextStage_->Init("clear_nextStage");
    nextStage_->CreateModel("clear/nextStage.obj");

    backSelect_ = std::make_unique<BaseObject>();
    backSelect_->Init("clear_backSelect");
    backSelect_->CreateModel("clear/backSelect.obj");

    retry_ = std::make_unique<BaseObject>();
    retry_->Init("clear_restart");
    retry_->CreateModel("clear/Retry.obj");

    animaChara_ = std::make_unique<BaseObject>();
    animaChara_->Init("animaChara");
    animaChara_->CreateModel("animation/playerClear.gltf");
    if (isPlayerWhite) {
        animaChara_->SetTexture("game/playerWhite.png");
    } else {
        animaChara_->SetTexture("game/player.png");
    }
    animaChara_->AddAnimation("animation/playerClearAfter2.gltf");

    animaGoal_ = std::make_unique<BaseObject>();
    animaGoal_->Init("animaGoal");
    animaGoal_->CreateModel("animation/goalClear.gltf");
    animaGoal_->AddAnimation("animation/goalClearAfter2.gltf");

    input_ = Input::GetInstance();
    if (stageNum_ < 15) {
        currentItem_ = 0;
    } else {
        currentItem_ = 1;
    }
    isDecision_ = false;

    decisionEmitter_ = ParticleEditor::GetInstance()->GetEmitter("clearDesition");
    coinEmitter_ = ParticleEditor::GetInstance()->GetEmitter("clearcoin");

    InitNumbers();

    coins_.resize(3);
    for (size_t index = 0; index < coins_.size(); index++) {
        coins_[index] = std::make_unique<BaseObject>();
        coins_[index]->Init("coin" + std::to_string(index + 1));
        coins_[index]->CreateModel("game/coinFlame.obj");
        t_[index] = 0.0f;
    }
    coinSE_ = Audio::GetInstance()->LoadWave("action/coinResult.wav");
    desitionSE_ = Audio::GetInstance()->LoadWave("select/stageDesition.wav");
    selectSE_ = Audio::GetInstance()->LoadWave("select/stageSelect.wav");
}

void ClearUI::Update() {
    book_->Update();
    stage_->Update();
    nextStage_->Update();
    backSelect_->Update();
    retry_->Update();
    singleDigit_->Update();
    twoDigit_->Update();
    if (stageNum_ > 14) {
        backSelect_->SetWorldPositionY(-8.1f);
        retry_->SetWorldPositionY(-7.4f);
    }

    if (!isDecision_) {
        MenuOperation();
    }
    if (input_->TriggerKey(DIK_SPACE) && !isDecision_ && timer_ < 0) {
        decisionEmitter_->UpdateOnce();
        Audio::GetInstance()->PlayWave(desitionSE_, 0.2f);
    }
    XINPUT_STATE joyState;
    XINPUT_STATE prejoyState;
    if (input_->GetJoystickState(0, joyState) && input_->GetJoystickStatePrevious(0, prejoyState)) {
        if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) && (!prejoyState.Gamepad.wButtons) && !isDecision_ && timer_ < 0) {
            decisionEmitter_->UpdateOnce();
            Audio::GetInstance()->PlayWave(desitionSE_, 0.2f);
        }
    }
    MoveUI();

    if (animaChara_->AnimaIsFinish()) {
        animaChara_->SetAnima("animation/playerClearAfter2.gltf");
        animaChara_->SetLoop(true);
    }
    if (animaGoal_->AnimaIsFinish()) {
        animaGoal_->SetAnima("animation/goalClearAfter2.gltf");
        animaGoal_->SetLoop(true);
    }

    animaChara_->Update();
    animaGoal_->Update();

    for (auto &coin : coins_) {
        coin->Update();
    }
    CoinUpdate();
}

void ClearUI::Draw(const ViewProjection &vp) {
    book_->Draw(vp);
    stage_->Draw(vp);

    singleDigit_->Draw(vp);
    twoDigit_->Draw(vp);
    animaChara_->Draw(vp);
    animaGoal_->Draw(vp);

    for (auto &coin : coins_) {
        coin->Draw(vp);
    }
}

void ClearUI::DrawParticle(const ViewProjection &vp) {
    ParticleCommon::GetInstance()->SetBlendMode(BlendMode::kAdd);
    decisionEmitter_->Draw(vp);
    coinEmitter_->Draw(vp);
}

void ClearUI::DrawTexts(const ViewProjection &vp) {
    if (stageNum_ < 15) {
        nextStage_->Draw(vp);
    }
    backSelect_->Draw(vp);
    retry_->Draw(vp);
}

void ClearUI::Debug() {
    ImGui::Begin("Clear");
    ImGui::Text("選択してる項目 : %d", currentItem_);
    ImGui::SliderInt("コイン", &coinNum_, 0, 3);
    ImGui::End();
    book_->DebugImGui();
    stage_->DebugImGui();
    nextStage_->DebugImGui();
    backSelect_->DebugImGui();
    retry_->DebugImGui();
    singleDigit_->DebugImGui();
    twoDigit_->DebugImGui();
    decisionEmitter_->Debug();
    animaChara_->DebugImGui();
    animaGoal_->DebugImGui();
    for (auto &coin : coins_) {
        coin->DebugImGui();
    }
}

void ClearUI::MenuOperation() {
    if (timer_ > 0) {
        timer_ -= 1.0f / 60.0f;
        return;
    }

    if (input_->PushKey(DIK_W) && coolTime_ == 0.0f) {
        --currentItem_;
        coolTime_ = 0.2f;
        Audio::GetInstance()->PlayWave(selectSE_, 0.2f);
    }
    if (input_->PushKey(DIK_S) && coolTime_ == 0.0f) {
        ++currentItem_;
        coolTime_ = 0.2f;
        Audio::GetInstance()->PlayWave(selectSE_, 0.2f);
    }

    XINPUT_STATE joyState;
    if (input_->GetJoystickState(0, joyState)) {
        if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP || joyState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f) {
            --currentItem_;
            coolTime_ = 0.2f;
            Audio::GetInstance()->PlayWave(selectSE_, 0.2f);
        }
        if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN || joyState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f) {
            ++currentItem_;
            coolTime_ = 0.2f;
            Audio::GetInstance()->PlayWave(selectSE_, 0.2f);
        }
    }

    if (coolTime_ > 0.0f) {
        coolTime_ -= 1.0f / 60.0f;
    }
    if (coolTime_ < 0.0f) {
        coolTime_ = 0.0f;
    }

    if (stageNum_ < 15) {
        if (currentItem_ < 0) {
            currentItem_ = 2;
        }
        if (currentItem_ > 2) {
            currentItem_ = 0;
        }
    } else {
        if (currentItem_ < 1) {
            currentItem_ = 2;
        }
        if (currentItem_ > 2) {
            currentItem_ = 1;
        }
    }
}

void ClearUI::InitNumbers() {
    singleDigit_ = std::make_unique<BaseObject>();
    twoDigit_ = std::make_unique<BaseObject>();

    singleDigit_->Init("singleDigit");
    twoDigit_->Init("twoDigit");

    // 一桁目のモデルを設定
    int singleDigitValue = stageNum_ % 10;
    std::string singleDigitModelPath = "clear/" + std::to_string(singleDigitValue) + ".obj";
    singleDigit_->CreateModel(singleDigitModelPath);

    // 二桁目のモデルを設定
    int twoDigitValue = stageNum_ / 10;
    if (twoDigitValue > 0) {
        std::string twoDigitModelPath = "clear/" + std::to_string(twoDigitValue) + ".obj";
        twoDigit_->CreateModel(twoDigitModelPath);
    } else {
        // 二桁目がない場合は0を表すモデルを設定
        twoDigit_->CreateModel("clear/0.obj");
    }
}

void ClearUI::MoveUI() {
    const float easeTMax = 1.0f;
    const Vector3 startScale = {0.5f, 0.5f, 0.5f};
    const Vector3 endScale = {0.6f, 0.6f, 0.6f};
    const float deltaTime = 1.0f / 60.0f;

    // 選択されているUI要素の拡縮アニメーション
    if (currentItem_ == 0) {
        decisionEmitter_->SetPositionY(-7.0f);
        // decisionEmitter_->SetScale({ 1.7f,0.2f,0.0f });
        nextStage_->SetTexture("clear/UI2_1x1.png");
        if (!isDecision_) {
            nextT_ += deltaTime;
            if (nextT_ > easeTMax) {
                nextT_ -= easeTMax; // ループさせるために初期化
            }
            nextStage_->SetScale(EaseInOutSine<Vector3>(startScale, endScale, nextT_, easeTMax));
        } else {
            nextT_ += deltaTime;
            if (nextT_ > easeTMax) {
                nextT_ = easeTMax;
            }
            nextStage_->SetScale(EaseInSine<Vector3>(nextStage_->GetTransform().scale_, endScale, nextT_, easeTMax));
        }

    } else {
        nextStage_->SetTexture("clear/UI1x1.png");
        if (nextT_ > 0.0f) {
            nextT_ -= deltaTime;
        } else {
            nextT_ = 0.0f;
        }
        nextStage_->SetScale(EaseInOutSine<Vector3>(startScale, nextStage_->GetTransform().scale_, nextT_, easeTMax)); // 選択されていない場合は縮小
    }

    if (currentItem_ == 1) {
        if (stageNum_ < 15) {
            decisionEmitter_->SetPositionY(-7.7f);
        } else {
            decisionEmitter_->SetPositionY(-7.4f);
        }
        // decisionEmitter_->SetScale({ 1.2f,0.2f,0.0f });
        retry_->SetTexture("clear/UI2_1x1.png");
        if (!isDecision_) {

            retryT_ += deltaTime;
            if (retryT_ > easeTMax) {
                retryT_ -= easeTMax; // ループさせるために初期化
            }
            retry_->SetScale(EaseInOutSine<Vector3>(startScale, endScale, retryT_, easeTMax));
        } else {
            retryT_ += deltaTime;
            if (retryT_ > easeTMax) {
                retryT_ = easeTMax;
            }
            retry_->SetScale(EaseInSine<Vector3>(retry_->GetTransform().scale_, endScale, retryT_, easeTMax));
        }
    } else {
        retry_->SetTexture("clear/UI1x1.png");
        if (retryT_ > 0.0f) {
            retryT_ -= deltaTime;
        } else {
            retryT_ = 0.0f;
        }
        retry_->SetScale(EaseInOutSine<Vector3>(startScale, retry_->GetTransform().scale_, retryT_, easeTMax)); // 選択されていない場合は縮小
    }

    if (currentItem_ == 2) {
        if (stageNum_ < 15) {
            decisionEmitter_->SetPositionY(-8.4f);
        } else {
            decisionEmitter_->SetPositionY(-8.1f);
        }
        // decisionEmitter_->SetScale({ 1.4f,0.2f,0.0f });
        backSelect_->SetTexture("clear/UI2_1x1.png");
        if (!isDecision_) {
            selectT_ += deltaTime;
            if (selectT_ > easeTMax) {
                selectT_ -= easeTMax; // ループさせるために初期化
            }
            backSelect_->SetScale(EaseInOutSine<Vector3>(startScale, endScale, selectT_, easeTMax));
        } else {
            selectT_ += deltaTime;
            if (selectT_ > easeTMax) {
                selectT_ = easeTMax;
            }
            backSelect_->SetScale(EaseInSine<Vector3>(backSelect_->GetTransform().scale_, endScale, selectT_, easeTMax));
        }
    } else {
        backSelect_->SetTexture("clear/UI1x1.png");
        if (selectT_ > 0.0f) {
            selectT_ -= deltaTime;
        } else {
            selectT_ = 0.0f;
        }
        backSelect_->SetScale(EaseInOutSine<Vector3>(startScale, backSelect_->GetTransform().scale_, selectT_, easeTMax)); // 選択されていない場合は縮小
    }
}

void ClearUI::CoinUpdate() {
    // アニメーション中のコインがcoinNum_より大きいならアニメーション終了
    if (currentCoinIndex_ >= coinNum_) {
        return;
    }

    // アニメーション用の共通パラメータ
    float startY = initialY_;    // 初期Y座標
    float peakY = startY + 0.5f; // 上昇する高さ
    float startRot = 0.0f;       // 初期回転
    float endRot = 1080.0f;      // 3回転

    // 各コインのアニメーションを更新
    for (int i = 0; i <= currentCoinIndex_ && i < coinNum_; i++) {
        if (t_[i] >= tMax_) {
            continue; // このコインのアニメーションは終了済み
        }

        float currentY;
        // 前半（0.0f～0.5f）: 上昇
        // 後半（0.5f～1.0f）: 下降
        if (t_[i] < tMax_ * 0.5f) {
            // 上昇処理（0～0.5の間）
            float normalizedT = t_[i] / (tMax_ * 0.5f); // 0～1の範囲に正規化
            currentY = EaseOutQuad<float>(startY, peakY, normalizedT, 1.0f);
        } else {
            // 下降処理（0.5～1.0の間）
            float normalizedT = (t_[i] - tMax_ * 0.5f) / (tMax_ * 0.5f); // 0～1の範囲に正規化
            currentY = EaseOutQuad<float>(peakY, startY, normalizedT, 1.0f);
        }

        // 回転の処理
        float easedRot = EaseInSine<float>(startRot, endRot, t_[i], tMax_);

        // Y座標と回転を適用
        coins_[i]->SetWorldPositionY(currentY);
        coins_[i]->SetRotationY(degreesToRadians(easedRot));

        // 全体の回転の半分を超えたら色を黄色にする
        if (easedRot >= endRot / 2.0f) {
            // まだ黄色になっていないコインの場合
            if (!isPlayedSE_[i]) {
                coins_[i]->SetModel("game/coin.obj");
                coins_[i]->SetTexture("game/coin.png");
                Audio::GetInstance()->PlayWave(coinSE_, 0.2f);
                isPlayedSE_[i] = true; // 効果音再生済みフラグを立てる
            }
        }

        // このコインのアニメーション時間を進める
        t_[i] += deltaTime_;

        // アニメーション終了処理
        if (t_[i] >= tMax_) {
            coins_[i]->SetWorldPositionY(startY);
            coins_[i]->SetRotationY(degreesToRadians(startRot));
            coinEmitter_->SetPosition(coins_[i]->GetCenterPosition());
            coinEmitter_->UpdateOnce();
        }
    }

    // 次のコインのアニメーション開始判定
    if (currentCoinIndex_ < coinNum_ - 1) {          // 最後のコイン以外で
        if (t_[currentCoinIndex_] > tMax_ * 0.75f) { // 現在のコインが50%を超えたら
            currentCoinIndex_++;                     // 次のコインを開始
        }
    }
}