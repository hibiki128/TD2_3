#include "UIObject.h"

void UIObject::Init() {
	objectBook_ = std::make_unique<BaseObject>(); 
	objectBook_->CreateModel("game/openBook.obj");
	objectBook_->SetTexture("game/openBook.png");

	// ゴリ押しで位置決め
	objectBook_->SetWorldPosition({12.690f, -5.330f, 7.870f});
	objectBook_->SetRotation({-1.570f, 0.0f, 0.0f});
	objectBook_->SetScale({4.50f, 4.50f, 4.50f});

	objectUI_ = std::make_unique<BaseObject>();
	objectUI_->CreateModel("game/UI.obj");
	objectUI_->SetTexture("game/ui.png");

	// ゴリ押しで位置決め
	objectUI_->SetWorldPosition({-10.620f, -9.45f, 0.0f});
	objectUI_->SetRotation({-1.570f, 0.0f, 0.0f});
	objectUI_->SetScale({2.0f, 2.0f, 2.0f});

	objectL_ = std::make_unique<BaseObject>();
	objectL_->CreateModel("game/L.obj");
	objectL_->SetTexture("game/L.png");

	// ゴリ押しで位置決め
	objectL_->SetWorldPosition({-10.63f, -10.110f, 0.0f});
	objectL_->SetRotation({-1.570f, 0.0f, 0.0f});
	objectL_->SetScale({2.5f, 2.5f, 2.5f});

	objectA_ = std::make_unique<BaseObject>();
	objectA_->CreateModel("game/A.obj");
	objectA_->SetTexture("game/A.png");

	// ゴリ押しで位置決め
	objectA_->SetWorldPosition({-10.63f, -9.41f, 0.0f});
	objectA_->SetRotation({-1.570f, 0.0f, 0.0f});
	objectA_->SetScale({2.0f, 2.0f, 2.0f});

	objectR_ = std::make_unique<BaseObject>();
	objectR_->CreateModel("game/R.obj");
	objectR_->SetTexture("game/R.png");

	// ゴリ押しで位置決め
	objectR_->SetWorldPosition({-10.63f, -9.59f, 0.0f});
	objectR_->SetRotation({-1.570f, 0.0f, 0.0f});
	objectR_->SetScale({2.0f, 2.0f, 2.0f});
}

void UIObject::Update() { 
	objectBook_->Update(); 
	objectUI_->Update();
	objectL_->Update();
	objectA_->Update();
	objectR_->Update();
}

void UIObject::Draw(const ViewProjection& viewProjection) { 
	objectBook_->Draw(viewProjection); 
	objectUI_->Draw(viewProjection);
	objectL_->Draw(viewProjection);
	objectA_->Draw(viewProjection);
	objectR_->Draw(viewProjection);
}
