#include "TitleScene.h"
#include "ImGuiManager.h"
#include"SceneManager.h"
#include"SrvManager.h"

#ifdef _DEBUG
#include<imgui.h>
#endif // _DEBUG
#include <LightGroup.h>
#include"line/DrawLine3D.h"

void TitleScene::Initialize()
{
	audio_ = Audio::GetInstance();
	objCommon_ = Object3dCommon::GetInstance();
	spCommon_ = SpriteCommon::GetInstance();
	ptCommon_ = ParticleCommon::GetInstance();
	input_ = Input::GetInstance();
	vp_.Initialize();
	vp_.translation_ = { 12.0f,-4.0f,-30.0f };

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(&vp_);

	///
	///	オブジェクト生成
	/// 
	
	// マップチップフィールド
	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->Init("resources/Maps/stage1.csv");

	// プレイヤー（マップチップフィールドから初期位置を取得するので後）
	player_ = std::make_unique<Player>();
	player_->Init("player");
	player_->SetInitialPosition(mapChipField_->GetPlayerInitialPosition()); // csvから読み込んだ初期位置を設定

	// タイトルUIオブジェクト生成
	objectTitleUI_ = std::make_unique<TitleUI>();
	objectTitleUI_->Init();

	BGM_ = audio_->LoadWave("title/titleBgm.wav");
	audio_->PlayWave(BGM_, 0.2f, true);

	///
	///	スプライト生成
	/// 
	
	spriteBackGround_ = std::make_unique<Sprite>();
	spriteBackGround_->Initialize("title/backGround.png", { 0.0f, 0.0f });
}

void TitleScene::Finalize()
{
	audio_->StopWave(BGM_);
}

void TitleScene::Update()
{
	///
	///	オブジェクト更新
	///	

	objectTitleUI_->Update();

	// プレイヤー更新
	player_->Update(mapChipField_.get());

	// マップチップフィールド更新
	mapChipField_->Update(player_->GetCenterPosition(), player_->GetInvertRangeX(), player_->GetInvertRangeY());

	player_->Reset();
	player_->PlaySE();
	mapChipField_->PlaySE();

#ifdef _DEBUG
	// デバッグ
	Debug();
#endif // _DEBUG

	// カメラ更新
	CameraUpdate();

	// シーン切り替え
	ChangeScene();


}

void TitleScene::Draw()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----

	spriteBackGround_->Draw(true);

	objCommon_->DrawCommonSetting();
	//-----3DObjectの描画開始-----

	objectTitleUI_->Draw(vp_);

	// プレイヤー描画
	player_->Draw(vp_);

	// マップチップフィールド描画
	mapChipField_->Draw(vp_);

	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------
	mapChipField_->DrawParticle(vp_);

	//-----------------------------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----
	player_->DrawSprite(vp_);

	//------------------------------
	
	//-----線描画-----
	DrawLine3D::GetInstance()->Draw(vp_);
	//---------------

	/// ----------------------------------

	/// -------描画処理終了-------
}

void TitleScene::DrawForOffScreen()
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


void TitleScene::Debug()
{
	ImGui::Begin("TitleScene:Debug");
	debugCamera_->imgui();
	LightGroup::GetInstance()->imgui();
	ImGui::End();
	objectTitleUI_->Debug();
	player_->DebugImGui();
}

void TitleScene::CameraUpdate()
{
	if (debugCamera_->GetActive()) {
		debugCamera_->Update();
	}
	else {
		vp_.UpdateMatrix();
	}
}

void TitleScene::ChangeScene()
{
	if(player_->IsGoalReached()){
		sceneManager_->NextSceneReservation("SELECT");
	}
}
