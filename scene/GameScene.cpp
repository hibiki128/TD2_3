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

	// プレイヤー
	player_ = std::make_unique<Player>();
	player_->Init("player");

	// マップチップフィールド
	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->Init("resources/Maps/stage1.csv");

	// ポーズ
	pause_ = std::make_unique<Pause>();
	pause_->Init();

	///
	///	スプライト初期化
	/// 

	// 操作説明スプライト
	spriteGuide_ = std::make_unique<Sprite>();
	spriteGuide_->Initialize(
		"temp_guide.png", 
		{260.0f, 660.0f}, 
		{1.0f, 1.0f, 1.0f, 1.0f}, 
		{0.5f, 0.5f}
	);
}

void GameScene::Update()
{
#ifdef _DEBUG
	// デバッグ
	Debug();
#endif // _DEBUG

	// カメラ更新
	CameraUpdate();

	// シーン切り替え
	ChangeScene();

	///
	///	各オブジェクト更新
	/// 

	// プレイヤー更新
	player_->Update(mapChipField_.get());

	// マップチップフィールド更新
	mapChipField_->Update();

	// ポーズ更新
	pause_->Update();
}

void GameScene::Draw()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----

	// 操作説明スプライトの描画
	spriteGuide_->Draw();

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

	// ポーズ描画
	pause_->Draw(vp_);

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

	//-----------------------------


	/// ----------------------------------

	/// -------描画処理終了-------
}

void GameScene::Debug()
{
	ImGui::Begin("GameScene:Debug");
	debugCamera_->imgui();
	LightGroup::GetInstance()->imgui();
	ImGui::End();

	// プレイヤーデバッグ情報
	player_->DebugImGui();
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
	/*if (input_->TriggerKey(DIK_SPACE)) {
		sceneManager_->NextSceneReservation("TITLE");
	}*/
}
