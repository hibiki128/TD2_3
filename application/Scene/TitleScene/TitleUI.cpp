#include "TitleUI.h"
#include"SpriteCommon.h"

/// タイトルの仕様書**************************************************************************
/// https://docs.google.com/document/d/1HWlYKVP7claXDMJmXlaweO116KhdEbmu_mTf-6ORF1I/edit?tab=t.hs6t3nu61xqa#heading=h.jcw0f0b1h3x5
///*****************************************************************************************


void TitleUI::Init()
{
	/// フラグ初期化
	isStart_ = false;

	start_ = std::make_unique<BaseObject>();
	start_->CreateModel("title/titleStart.obj");
	start_->SetTexture("title/title.png");

	title_ = std::make_unique<BaseObject>();
	title_->CreateModel("title/title.obj");
	title_->SetTexture("title/title.png");
}

void TitleUI::Update()
{
	/// 各オブジェクト更新
	title_->Update();
	start_->Update();
}

void TitleUI::Draw(const ViewProjection& vp)
{
	/// 各オブジェクト描画
	title_->Draw(vp);
	start_->Draw(vp);

	/// 各スプライト描画
	SpriteCommon::GetInstance()->DrawCommonSetting();
}

void TitleUI::Debug()
{
	/// 各オブジェクトデバッグ描画
	title_->DebugImGui();
	start_->DebugImGui();

	ImGui::Begin("TitleUI");
	ImGui::End();
}
