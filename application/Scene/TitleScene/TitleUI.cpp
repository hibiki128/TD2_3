#include "TitleUI.h"
#include"SpriteCommon.h"

/// タイトルの仕様書**************************************************************************
/// https://docs.google.com/document/d/1HWlYKVP7claXDMJmXlaweO116KhdEbmu_mTf-6ORF1I/edit?tab=t.hs6t3nu61xqa#heading=h.jcw0f0b1h3x5
///*****************************************************************************************


void TitleUI::Init()
{
	/// フラグ初期化
	isStart_ = false;

	/// 各値初期化
	buttonA_pos = { 0.0f,0.0f };

	/// インスタンス生成
	start_ = std::make_unique<BaseObject>();
	title_ = std::make_unique<BaseObject>();
	buttonA_ = std::make_unique<Sprite>();

	/// 各オブジェクト初期化
	start_->Init("start");
	start_->CreateModel("title/start.obj");
	title_->Init("title");
	title_->CreateModel("title/title.obj");
	buttonA_->Initialize("title/buttonA.png", buttonA_pos, { 1.0f,1.0f,1.0f,1.0f }, { 0.5f,0.5f });
}

void TitleUI::Update()
{
	/// 各オブジェクト更新
	title_->Update();
	start_->Update();
	buttonA_->SetPosition(buttonA_pos);
}

void TitleUI::Draw(const ViewProjection& vp)
{
	/// 各オブジェクト描画
	title_->Draw(vp);
	start_->Draw(vp);

	/// 各スプライト描画
	SpriteCommon::GetInstance()->DrawCommonSetting();
	buttonA_->Draw();
}

void TitleUI::Debug()
{
	/// 各オブジェクトデバッグ描画
	title_->DebugImGui();
	start_->DebugImGui();

	ImGui::Begin("TitleUI");
	ImGui::DragFloat2("ボタンスプライトの位置", &buttonA_pos.x, 1.0f);
	ImGui::End();
}
