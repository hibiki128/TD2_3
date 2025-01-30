#include "SelectUI.h"


void SelectUI::Init()
{
	backTitle_ = std::make_unique<Sprite>();
	backTitle_->Initialize("select/backTitle.png", { 0.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f }, { 0.5f,0.5f });
	decision_ = std::make_unique<Sprite>();
	decision_->Initialize("select/decision.png", { 0.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f }, { 0.5f,0.5f });

	pos_ = { 240.0f,660.0f };
	size_ = backTitle_->GetSize();
	size2_ = decision_->GetSize();
	value_ = 0.85f;
}

void SelectUI::Update()
{
	backTitle_->SetPosition({190.0f,670.0f});
	backTitle_->SetSize(size_ * 0.65f);
	decision_->SetPosition({ 640.0f, 620.0f});
	decision_->SetSize(size2_ * 0.65f);
}

void SelectUI::Draw()
{
	backTitle_->Draw();
	decision_->Draw();
}