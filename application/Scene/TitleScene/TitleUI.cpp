#include "TitleUI.h"
#include"SpriteCommon.h"
#include"Easing.h"
#include"myEngine/Frame/Frame.h"

/// タイトルの仕様書**************************************************************************
/// https://docs.google.com/document/d/1HWlYKVP7claXDMJmXlaweO116KhdEbmu_mTf-6ORF1I/edit?tab=t.hs6t3nu61xqa#heading=h.jcw0f0b1h3x5
///*****************************************************************************************


void TitleUI::Init()
{
	start_ = std::make_unique<BaseObject>();
	start_->Init("start");
	start_->CreateModel("title/titleStart.obj");
	start_->SetTexture("title/title.png");

	title_ = std::make_unique<BaseObject>();
	title_->Init("title");
	title_->CreateModel("title/title.obj");
	title_->SetTexture("title/title.png");

	t_ = 0.0f;
	startTime_ = 1.0f;
}

void TitleUI::Update()
{
	/// 各オブジェクト更新
	title_->Update();
	start_->Update();
	TextMove();
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
}

void TitleUI::TextMove()
{
	const float startPos = 20.0f;
	const float endPos = 6.0f;
	const float easeTMax = 3.0f;

	if (startTime_ <= 0) {
		if (t_ < easeTMax) {
			t_ += Frame::DeltaTime();
		}
		else {
			t_ = easeTMax;
		}
		startTime_ = 0;
	}
	else {
		startTime_ -= Frame::DeltaTime();
	}

	title_->SetWorldPositionY(EaseOutBounce<float>(startPos, endPos, t_, easeTMax));

}
