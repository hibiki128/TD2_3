#include "ClearScene.h"
#include "SceneManager.h"
#include "externals/nlohmann/json.hpp"
#include <LightGroup.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <line/DrawLine3D.h>

void ClearScene::Finalize() {
    InitFilePath();

#ifndef _DEBUG
    StageDataForJson();
#endif

    sceneManager_->SetFilePath(filePath_);
    audio_->StopWave(BGM_);
}

void ClearScene::Initialize() {

    audio_ = Audio::GetInstance();
    objCommon_ = Object3dCommon::GetInstance();
    spCommon_ = SpriteCommon::GetInstance();
    ptCommon_ = ParticleCommon::GetInstance();
    input_ = Input::GetInstance();
    vp_.Initialize();
    vp_.translation_ = {12.0f, -6.5f, -30.0f};

    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize(&vp_);

    // コインの数を取得
    coinNum_ = sceneManager_->GetCoin();

    clearUI_ = std::make_unique<ClearUI>();
    clearUI_->SetStageNum(GetStageNum());
    clearUI_->Init();

    clearUI_->SetCoin(coinNum_);

    BGM_ = audio_->LoadWave("clear/clearBgm.wav");
    audio_->PlayWave(BGM_, 0.2f, true);

    ///
    ///	スプライト生成
    ///

    spriteBackGround_ = std::make_unique<Sprite>();
    spriteBackGround_->Initialize("title/backGround.png", {0.0f, 0.0f});
}

void ClearScene::Update() {
#ifdef _DEBUG
    // デバッグ
    Debug();
#endif // _DEBUG

    // カメラ更新
    CameraUpdate();

    ///
    ///	各オブジェクト更新
    ///

    clearUI_->Update();

    // シーン切り替え
    ChangeScene();
}

void ClearScene::Draw() {
    /// -------描画処理開始-------

    /// Spriteの描画準備
    spCommon_->DrawCommonSetting();
    //-----Spriteの描画開始-----

    // 背景
    spriteBackGround_->Draw(true);

    //------------------------

    objCommon_->DrawCommonSetting();
    //-----3DObjectの描画開始-----

    ///
    ///	各オブジェクト描画
    ///

    clearUI_->Draw(vp_);

    //--------------------------

    /// Particleの描画準備
    ptCommon_->DrawCommonSetting();
    //------Particleの描画開始-------
    clearUI_->DrawParticle(vp_);
    //-----------------------------

    objCommon_->DrawCommonSetting();
    //-----3DObjectの描画開始-----

    clearUI_->DrawTexts(vp_);

    //--------------------------

    /// Spriteの描画準備
    spCommon_->DrawCommonSetting();
    //-----Spriteの描画開始-----

    //------------------------

    //-----線描画-----
#ifdef _DEBUG
    DrawLine3D::GetInstance()->Draw(vp_);
#endif // _DEBUG
    //---------------

    /// ----------------------------------

    /// -------描画処理終了-------
}

void ClearScene::DrawForOffScreen() {
    /// -------描画処理開始-------

    /// Spriteの描画準備
    spCommon_->DrawCommonSetting();
    //-----Spriteの描画開始-----

    //------------------------

    objCommon_->DrawCommonSetting();
    //-----3DObjectの描画開始-----

    //--------------------------

    /// Particleの描画準備
    ptCommon_->DrawCommonSetting();
    //------Particleの描画開始-------

    //-----------------------------

    /// ----------------------------------

    /// -------描画処理終了-------
}

void ClearScene::Debug() {
    ImGui::Begin("ClearScene:Debug");
    debugCamera_->imgui();
    LightGroup::GetInstance()->imgui();
    ImGui::End();
    clearUI_->Debug();
}

void ClearScene::CameraUpdate() {
    if (debugCamera_->GetActive()) {
        debugCamera_->Update();
    } else {
        vp_.UpdateMatrix();
    }
}

void ClearScene::ChangeScene() {
    XINPUT_STATE joyState;
    if (input_->GetJoystickState(0, joyState)) {
        if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) {
            clearUI_->SetDecision(true);
            if (clearUI_->GetItemNum() == 0 || clearUI_->GetItemNum() == 1) {
                sceneManager_->NextSceneReservation("GAME");
            }
            if (clearUI_->GetItemNum() == 2) {
                sceneManager_->NextSceneReservation("SELECT");
            }
        }
    }
    if (input_->TriggerKey(DIK_SPACE)) {
        clearUI_->SetDecision(true);
        if (clearUI_->GetItemNum() == 0 || clearUI_->GetItemNum() == 1) {
            sceneManager_->NextSceneReservation("GAME");
        }
        if (clearUI_->GetItemNum() == 2) {
            sceneManager_->NextSceneReservation("SELECT");
        }
    }
}

void ClearScene::InitFilePath() {
    // 現在のファイルパスを取得
    filePath_ = sceneManager_->GetFilePath();

    // 数字部分を探してインクリメントする
    size_t stagePos = filePath_.find("stage");
    if (stagePos != std::string::npos) {
        size_t numberStart = filePath_.find_first_of("0123456789", stagePos);
        if (numberStart != std::string::npos) {
            size_t numberEnd = filePath_.find_first_not_of("0123456789", numberStart);
            std::string numberStr = filePath_.substr(numberStart, numberEnd - numberStart);
            int stageNumber = std::stoi(numberStr); // 数字部分を取得
            if (clearUI_->GetItemNum() == 0) {
                ++stageNumber; // 数字を一つ進める
            }

            // 新しいファイルパスを生成
            filePath_ = filePath_.substr(0, numberStart) + std::to_string(stageNumber) + filePath_.substr(numberEnd);
        }
    }
}

int ClearScene::GetStageNum() {
    // 現在のファイルパスを取得
    filePath_ = sceneManager_->GetFilePath();
    int stageNumber;
    // 数字部分を探してインクリメントする
    size_t stagePos = filePath_.find("stage");
    if (stagePos != std::string::npos) {
        size_t numberStart = filePath_.find_first_of("0123456789", stagePos);
        if (numberStart != std::string::npos) {
            size_t numberEnd = filePath_.find_first_not_of("0123456789", numberStart);
            std::string numberStr = filePath_.substr(numberStart, numberEnd - numberStart);
            stageNumber = std::stoi(numberStr); // 数字部分を取得
        }
    }

    // #ifdef _DEBUG
    //	stageNumber = 1;
    // #endif // _DEBUG

    return stageNumber;
}

void ClearScene::StageDataForJson() {
    // ステージのファイルパスを取得
    std::string filePath = sceneManager_->GetFilePath();

    // ステージ番号を抽出
    std::size_t found = filePath.find_last_of("/\\");
    std::string fileName = filePath.substr(found + 1);
    std::string stageNumber = fileName.substr(5, fileName.find_last_of(".") - 5);

    // JSONオブジェクトを作成
    nlohmann::json jsonData = {
        {"CoinNum", coinNum_}};

    // StageDataフォルダのパス
    std::string stageDataFolderPath = "resources/jsons/StageData/";

    // フォルダが存在しない場合は作成
    if (!std::filesystem::exists(stageDataFolderPath)) {
        std::filesystem::create_directories(stageDataFolderPath);
    }

    // JSONファイルの保存
    std::string jsonFileName = stageDataFolderPath + "stage" + stageNumber + ".json";
    std::ofstream jsonFile(jsonFileName);
    if (jsonFile.is_open()) {
        jsonFile << jsonData.dump(4); // 4はインデントのスペース数
        jsonFile.close();
    }
}