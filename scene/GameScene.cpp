#include "GameScene.h"
#include "SceneManager.h"
#include <LightGroup.h>
#include <line/DrawLine3D.h>

void GameScene::Finalize() {
    sceneManager_->SetFilePath(filePath_);
    audio_->StopWave(BGM_);
}

void GameScene::Initialize() {

    audio_ = Audio::GetInstance();
    objCommon_ = Object3dCommon::GetInstance();
    spCommon_ = SpriteCommon::GetInstance();
    ptCommon_ = ParticleCommon::GetInstance();
    input_ = Input::GetInstance();
    vp_.Initialize();
    
    currentStageNum_ = GetStageNum(); // 選択ステージの格納
    AdjustCameraPositionForStageNum(); // 選択したステージによってカメラの初期位置を変更

    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize(&vp_);

    filePath_ = sceneManager_->GetFilePath();
//#ifdef _DEBUG
//    filePath_ = "resources/Maps/stage1.csv";
//#endif // DEBUG

    ///
    ///	各オブジェクト初期化
    ///

    // マップチップフィールド
    mapChipField_ = std::make_unique<MapChipField>();
    mapChipField_->Init(filePath_);

    // プレイヤー（マップチップフィールドから初期位置を取得するので後）
    player_ = std::make_unique<Player>();
    player_->Init("player", GetStageNum()); // 初期化時に現在のステージ数をセット
    player_->SetInitialPosition(mapChipField_->GetPlayerInitialPosition()); // csvから読み込んだ初期位置を設定
    // UIオブジェクト
    uiObject_ = std::make_unique<UIObject>();
    uiObject_->Init(GetStageNum()); // 初期化時に現在のステージ数をセット

    // クリアカメラ
    clearCamera_ = std::make_unique<ClearCamera>();

    ///
    ///	スプライト初期化
    ///

    // 背景
    spriteBackGround_ = std::make_unique<Sprite>();
    spriteBackGround_->Initialize("title/backGround.png", {0.0f, 0.0f});

    // Jsonから保存情報の読み込み
    LoadFromJson();

    BGM_ = audio_->LoadWave("game/gameBgm.wav");
    audio_->PlayWave(BGM_, 0.2f, true);

    clearCamera_->Init(&vp_);

    // ポーズ
    pause_ = std::make_unique<Pause>();
    pause_->SetStageNum(GetStageNum());
    pause_->Init();
    pause_->SetPlayer(player_.get());

    
    leaf_ = std::make_unique<ParticleEmitter>();
    leaf_->Initialize("leaf", "game/leaf.obj");

}

void GameScene::Update() {
#ifdef _DEBUG
    // デバッグ
    Debug();
#endif // _DEBUG

    ///
    ///	各オブジェクト更新
    ///

    if (!pause_->IsPause() && !clearCamera_->GetActive()) {
        if (pause_->IsFinish()) {
            // プレイヤー更新
            player_->Update(mapChipField_.get());
        }
#ifdef _DEBUG

        // プレイヤーがゴールに到達した際の処理
        if (player_->IsGoalReached()) {
            ImGui::Begin("GameScene:Debug");
            ImGui::Text("Goal");
            ImGui::End();
        }

#endif // _DEBUG
       // マップチップフィールド更新
        mapChipField_->Update(player_->GetCenterPosition(), player_->GetInvertRangeX(), player_->GetInvertRangeY());
    }
    player_->Reset();
    if (!clearCamera_->GetActive()) {
        player_->PlaySE();
        mapChipField_->PlaySE();
        // ポーズ更新
        pause_->Update();
    }
    player_->BaseUpdate();

    // UIObject更新
    uiObject_->Update();

    leaf_->Update();

    // カメラ更新
    CameraUpdate();

    // シーン切り替え
    ChangeScene();
}

void GameScene::Draw() {
    /// -------描画処理開始-------

    /// Spriteの描画準備
    spCommon_->DrawCommonSetting();
    //-----Spriteの描画開始-----

    // 背景
    spriteBackGround_->Draw(true);

    // UI（ポーズボタン）
    uiObject_->DrawSprite();

    // プレイヤーUI
    if (!clearCamera_->GetActive()) { // クリアカメラがアクティブになったら非表示にするため
        player_->DrawSprite(vp_);
    }

    //------------------------

    objCommon_->DrawCommonSetting();
    //-----3DObjectの描画開始-----

    ///
    ///	各オブジェクト描画
    ///

    // プレイヤー描画
    player_->Draw(vp_);

    // マップチップフィールド描画
    mapChipField_->Draw(vp_);

    // UIObject描画
    uiObject_->Draw(vp_);

    //--------------------------

    /// Particleの描画準備
    ptCommon_->DrawCommonSetting();
    //------Particleの描画開始-------
    mapChipField_->DrawParticle(vp_);
    ptCommon_->SetBlendMode(BlendMode::kNormal);
    leaf_->Draw(vp_);
    player_->DrawParticle(vp_);
    ptCommon_->SetBlendMode(BlendMode::kAdd);
    //-----------------------------

    /// Spriteの描画準備
    spCommon_->DrawCommonSetting();
    //-----Spriteの描画開始-----

    // ポーズ描画
    pause_->Draw(vp_);
    /*player_->DrawSprite(vp_);*/

    //------------------------

    //-----線描画-----
    // #ifdef _DEBUG
    DrawLine3D::GetInstance()->Draw(vp_);
    // #endif // _DEBUG
    //---------------

    /// ----------------------------------

    /// -------描画処理終了-------
}

void GameScene::DrawForOffScreen() {
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

void GameScene::Debug() {
    ImGui::Begin("GameScene:Debug");
    debugCamera_->imgui();
    LightGroup::GetInstance()->imgui();

    ///
    ///	追加分
    ///

    if (ImGui::BeginTabBar("Camera")) {
        if (ImGui::BeginTabItem("カメラ")) {

            // なんか追加する場合こっから
            ImGui::DragFloat3("位置", &vp_.translation_.x, 0.1f);

            if (ImGui::Button("セーブ")) {
                SaveToJson();
                std::string message = std::format("Camera saved.");
                MessageBoxA(nullptr, message.c_str(), "Object", 0);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();

    // プレイヤーデバッグ情報
    player_->DebugImGui();
    // マップチップフィールドデバッグ情報
    mapChipField_->DebugImGui();
    // UIObjectデバッグ情報
    uiObject_->DebugImGui();
    leaf_->imgui();
}

void GameScene::CameraUpdate() {
#ifdef _DEBUG
    /*if (debugCamera_->GetActive()) {
            debugCamera_->Update();
    }
    else {
            vp_.UpdateMatrix();
    }*/
#endif // _DEBUG

    if (player_->IsGoalReached()) {
        clearCamera_->SetActive(true);
    }
    if (clearCamera_->GetActive()) {
        clearCamera_->Update(player_->GetWorldPosition());
    } else {
        vp_.UpdateMatrix();
    }
}

void GameScene::ChangeScene() {
    // ゲームパッドの状態を取得
    XINPUT_STATE joyState;
    if (pause_->GetItem() == -2 &&
        (input_->TriggerKey(DIK_SPACE) ||
         (input_->GetJoystickState(0, joyState) && (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A)))) {
        // SELECTシーンへの遷移を予約
        sceneManager_->NextSceneReservation("SELECT");
        sceneManager_->SetCoinNum(0);
    }
    if (clearCamera_->GetFinish()&&player_->GetGoalAnimaFinish()) {
        sceneManager_->SetCoinNum(player_->GetCurrentCoinCount());
        sceneManager_->NextSceneReservation("CLEAR");
    }
}

void GameScene::SaveToJson() {
    json j;

    // なんか追加する場合こっから
    j["gravityAcceleration"] = {vp_.translation_.x, vp_.translation_.y, vp_.translation_.z};

    // ディレクトリを作成し、JSONファイルを保存
    std::filesystem::create_directories("resources/jsons/Camera/");
    std::ofstream outFile("resources/jsons/Camera/camera.json");
    outFile << j.dump(4);
}

void GameScene::LoadFromJson() {
    std::ifstream inFile("resources/jsons/Camera/camera.json");
    if (!inFile.is_open()) {
        return; // JSONファイルがない場合は早期リターン
    }

    json j;
    inFile >> j;

    // 各種JSONから読み込み
    if (j.contains("gravityAcceleration") && j["gravityAcceleration"].is_array()) {
        vp_.translation_ = {
            j["gravityAcceleration"][0], j["gravityAcceleration"][1], j["gravityAcceleration"][2]};
    }
}

void GameScene::AdjustCameraPositionForStageNum() {
    switch (currentStageNum_) {
    case -1:
        /*これがデフォルト値*/
        vp_.translation_ = {8.0f, -5.5f, -33.0f};

        break;
    case 1:
        vp_.translation_ = {8.0f, -5.5f, -33.0f};

        break;
    case 2:
        vp_.translation_ = {10.2f, -10.0f, -33.0f};

        break;
    case 3:
        vp_.translation_ = {12.4f, -5.5f, -33.0f};

        break;
    case 4:
        vp_.translation_ = {11.9f, -7.9f, -33.0f};

        break;
    case 5:
        vp_.translation_ = {8.0f, -10.0f, -33.0f};

        break;
    case 6:
        vp_.translation_ = {8.5f, -11.5f, -33.0f};

        break;
    case 7:
        vp_.translation_ = {12.8f, -12.4f, -33.0f};

        break;
    case 8:
        vp_.translation_ = {8.5f, -12.7f, -33.0f};

        break;
    case 9:
        vp_.translation_ = {11.7f, -10.5f, -33.0f};

        break;
    case 10:// ここから
        vp_.translation_ = {8.0f, -5.5f, -33.0f};

        break;
    case 11:
        vp_.translation_ = {8.0f, -5.5f, -33.0f};

        break;
    case 12:
        vp_.translation_ = {13.2f, -8.2f, -33.0f};

        break;
    case 13:
        vp_.translation_ = {8.0f, -9.7f, -33.0f};

        break;
    case 14:
        vp_.translation_ = {8.0f, -5.5f, -33.0f};

        break;
    case 15:
        vp_.translation_ = {8.0f, -8.9f, -33.0f};

        break;
    default:
        break;
    }
}

int GameScene::GetStageNum() {
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

//#ifdef _DEBUG
//    stageNumber = 1;
//#endif // _DEBUG

    return stageNumber;
}
