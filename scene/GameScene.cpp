#include "GameScene.h"
#include <LightGroup.h>
#include"SceneManager.h"
#include <line/DrawLine3D.h>

void GameScene::Finalize()
{

}

void GameScene::Initialize()
{

	audio_ = Audio::GetInstance();
	objCommon_ = Object3dCommon::GetInstance();
	spCommon_ = SpriteCommon::GetInstance();
	ptCommon_ = ParticleCommon::GetInstance();
	input_ = Input::GetInstance();
	vp_.Initialize();
	vp_.translation_ = { 12.0f, -6.5f, -30.0f };

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(&vp_);

	///
	///	各オブジェクト初期化
	/// 

	// マップチップフィールド
	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->Init("resources/Maps/stage1.csv");

	// プレイヤー（マップチップフィールドから初期位置を取得するので後）
	player_ = std::make_unique<Player>();
	player_->Init("player");
	player_->SetInitialPosition(mapChipField_->GetPlayerInitialPosition()); // csvから読み込んだ初期位置を設定
	// ポーズ
	pause_ = std::make_unique<Pause>();
	pause_->Init();
	pause_->SetPlayer(player_.get());

	///
	///	スプライト初期化
	/// 

	// Jsonから保存情報の読み込み
	LoadFromJson();
}

void GameScene::Update()
{
#ifdef _DEBUG
	// デバッグ
	Debug();
#endif // _DEBUG

	// カメラ更新
	CameraUpdate();

	///
	///	各オブジェクト更新
	/// 

	if (!pause_->IsPause()) {
		// プレイヤー更新
		player_->Update(mapChipField_.get());

		// プレイヤーがゴールに到達した際の処理
		if (player_->IsGoalReached()) {
			ImGui::Begin("GameScene:Debug");
			ImGui::Text("Goal");
			ImGui::End();
		}

		// マップチップフィールド更新
		mapChipField_->Update();
	}
	player_->Reset();
	// ポーズ更新
	pause_->Update();

	// シーン切り替え
	ChangeScene();
}

void GameScene::Draw()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----



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

	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------

	//-----------------------------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----
	
	// ポーズ描画
	pause_->Draw(vp_);
	player_->DrawSprite();

	//------------------------

	//-----線描画-----
	DrawLine3D::GetInstance()->Draw(vp_);
	//---------------

	/// ----------------------------------

	/// -------描画処理終了-------
}

void GameScene::DrawForOffScreen()
{
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
	mapChipField_->DrawParticle(vp_);
	//-----------------------------


	/// ----------------------------------

	/// -------描画処理終了-------
}

void GameScene::Debug()
{
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
}

void GameScene::CameraUpdate()
{
	if (debugCamera_->GetActive()) {
		debugCamera_->Update();
	}
	else {
		vp_.UpdateMatrix();
	}
}

void GameScene::ChangeScene()
{
	// ゲームパッドの状態を取得
	XINPUT_STATE joyState;
	if (pause_->GetItem() == -2 &&
		(input_->TriggerKey(DIK_SPACE) ||
			(input_->GetJoystickState(0, joyState) && (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A)))) {
		// SELECTシーンへの遷移を予約
		sceneManager_->NextSceneReservation("SELECT");
	}
	if (player_->IsGoalReached()) {
		sceneManager_->NextSceneReservation("SELECT");
	}
}

void GameScene::SaveToJson()
{
	json j;

	// なんか追加する場合こっから
	j["gravityAcceleration"] = { vp_.translation_.x, vp_.translation_.y, vp_.translation_.z };

	// ディレクトリを作成し、JSONファイルを保存
	std::filesystem::create_directories("resources/jsons/Camera/");
	std::ofstream outFile("resources/jsons/Camera/camera.json");
	outFile << j.dump(4);
}

void GameScene::LoadFromJson()
{
	std::ifstream inFile("resources/jsons/Camera/camera.json");
	if (!inFile.is_open()) {
		return; // JSONファイルがない場合は早期リターン
	}

	json j;
	inFile >> j;

	// 各種JSONから読み込み
	if (j.contains("gravityAcceleration") && j["gravityAcceleration"].is_array()) {
		vp_.translation_ = {
			j["gravityAcceleration"][0], j["gravityAcceleration"][1], j["gravityAcceleration"][2]
		};
	}
}
