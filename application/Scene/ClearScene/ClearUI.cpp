#include "ClearUI.h"
#include"Input.h"
#include"Easing.h"

void ClearUI::Init()
{
	book_ = std::make_unique<BaseObject>();
	book_->Init("book");
	book_->CreateModel("game/openBook.obj");

	stage_ = std::make_unique<BaseObject>();
	stage_->Init("clear_stage");
	stage_->CreateModel("clear/stage.obj");

	nextStage_ = std::make_unique<BaseObject>();
	nextStage_->Init("clear_nextStage");
	nextStage_->CreateModel("clear/nextStage.obj");

	backSelect_ = std::make_unique<BaseObject>();
	backSelect_->Init("clear_backSelect");
	backSelect_->CreateModel("clear/backSelect.obj");

	retry_ = std::make_unique<BaseObject>();
	retry_->Init("clear_restart");
	retry_->CreateModel("clear/Retry.obj");

	input_ = Input::GetInstance();
	currentItem_ = 0;
	isDecision_ = false;

	decisionEmitter_ = std::make_unique<ParticleEmitter>();
	decisionEmitter_->Initialize("clearDesition", "debug/sphere.obj");
	decisionEmitter_->SetTexture("clear/UI2_1x1.png");

	InitNumbers();
}

void ClearUI::Update()
{
	book_->Update();
	stage_->Update();
	nextStage_->Update();
	backSelect_->Update();
	retry_->Update();
	singleDigit_->Update();
	twoDigit_->Update();
	if (!isDecision_) {
		MenuOperation();
	}
	if (input_->TriggerKey(DIK_SPACE) && !isDecision_) {
		decisionEmitter_->UpdateOnce();
	}
	XINPUT_STATE joyState;
	XINPUT_STATE prejoyState;
	if (input_->GetJoystickState(0, joyState) && input_->GetJoystickStatePrevious(0, prejoyState)) {
		if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) && (!prejoyState.Gamepad.wButtons)&&!isDecision_) {
			decisionEmitter_->UpdateOnce();
		}
	}
	MoveUI();
}

void ClearUI::Draw(const ViewProjection& vp)
{
	book_->Draw(vp);
	stage_->Draw(vp);

	singleDigit_->Draw(vp);
	twoDigit_->Draw(vp);
}

void ClearUI::DrawParticle(const ViewProjection& vp)
{
	ParticleCommon::GetInstance()->SetBlendMode(BlendMode::kAdd);
	decisionEmitter_->Draw(vp);
}

void ClearUI::DrawTexts(const ViewProjection& vp)
{
	nextStage_->Draw(vp);
	backSelect_->Draw(vp);
	retry_->Draw(vp);
}

void ClearUI::Debug()
{
	ImGui::Begin("Clear");
	ImGui::Text("選択してる項目 : %d", currentItem_);
	ImGui::End();
	book_->DebugImGui();
	stage_->DebugImGui();
	nextStage_->DebugImGui();
	backSelect_->DebugImGui();
	retry_->DebugImGui();
	singleDigit_->DebugImGui();
	twoDigit_->DebugImGui();
	decisionEmitter_->imgui();
}

void ClearUI::MenuOperation()
{
	if (input_->PushKey(DIK_W) && coolTime_ == 0.0f)
	{
		--currentItem_;
		coolTime_ = 0.2f;
	}
	if (input_->PushKey(DIK_S) && coolTime_ == 0.0f)
	{
		++currentItem_;
		coolTime_ = 0.2f;
	}

	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState))
	{
		if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP || joyState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f)
		{
			--currentItem_;
			coolTime_ = 0.2f;
		}
		if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN || joyState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f)
		{
			++currentItem_;
			coolTime_ = 0.2f;
		}
	}

	if (coolTime_ > 0.0f) {
		coolTime_ -= 1.0f / 60.0f;
	}
	if (coolTime_ < 0.0f) {
		coolTime_ = 0.0f;
	}

	if (currentItem_ < 0)
	{
		currentItem_ = 2;
	}
	if (currentItem_ > 2)
	{
		currentItem_ = 0;
	}
}

void ClearUI::InitNumbers()
{
	singleDigit_ = std::make_unique<BaseObject>();
	twoDigit_ = std::make_unique<BaseObject>();

	singleDigit_->Init("singleDigit");
	twoDigit_->Init("twoDigit");

	// 一桁目のモデルを設定
	int singleDigitValue = stageNum_ % 10;
	std::string singleDigitModelPath = "clear/" + std::to_string(singleDigitValue) + ".obj";
	singleDigit_->CreateModel(singleDigitModelPath);

	// 二桁目のモデルを設定
	int twoDigitValue = stageNum_ / 10;
	if (twoDigitValue > 0)
	{
		std::string twoDigitModelPath = "clear/" + std::to_string(twoDigitValue) + ".obj";
		twoDigit_->CreateModel(twoDigitModelPath);
	}
	else
	{
		// 二桁目がない場合は0を表すモデルを設定
		twoDigit_->CreateModel("clear/0.obj");
	}
}

void ClearUI::MoveUI()
{
	const float easeTMax = 1.0f;
	const Vector3 startScale = { 0.5f, 0.5f, 0.5f };
	const Vector3 endScale = { 0.6f, 0.6f, 0.6f };
	const float deltaTime = 1.0f / 60.0f;

	// 選択されているUI要素の拡縮アニメーション
	if (currentItem_ == 0) {
		decisionEmitter_->SetPositionY(-7.0f);
		//decisionEmitter_->SetScale({ 1.7f,0.2f,0.0f });
		nextStage_->SetTexture("clear/UI2_1x1.png");
		if (!isDecision_) {
			nextT_ += deltaTime;
			if (nextT_ > easeTMax) {
				nextT_ -= easeTMax; // ループさせるために初期化
			}
			nextStage_->SetScale(EaseInOutSine<Vector3>(startScale, endScale, nextT_, easeTMax));
		}
		else {
			nextT_ += deltaTime;
			if (nextT_ > easeTMax) {
				nextT_ = easeTMax;
			}
			nextStage_->SetScale(EaseInSine<Vector3>(nextStage_->GetTransform().scale_, endScale, nextT_, easeTMax));
		}

	}
	else {
		nextStage_->SetTexture("clear/UI1x1.png");
		if (nextT_ > 0.0f) {
			nextT_ -= deltaTime;
		}
		else {
			nextT_ = 0.0f;
		}
		nextStage_->SetScale(EaseInOutSine<Vector3>(startScale, nextStage_->GetTransform().scale_, nextT_, easeTMax)); // 選択されていない場合は縮小
	}

	if (currentItem_ == 1) {
		decisionEmitter_->SetPositionY(-7.7f);
		//decisionEmitter_->SetScale({ 1.2f,0.2f,0.0f });
		retry_->SetTexture("clear/UI2_1x1.png");
		if (!isDecision_) {

			retryT_ += deltaTime;
			if (retryT_ > easeTMax) {
				retryT_ -= easeTMax; // ループさせるために初期化
			}
			retry_->SetScale(EaseInOutSine<Vector3>(startScale, endScale, retryT_, easeTMax));
		}
		else {
			retryT_ += deltaTime;
			if (retryT_ > easeTMax) {
				retryT_ = easeTMax;
			}
			retry_->SetScale(EaseInSine<Vector3>(retry_->GetTransform().scale_, endScale, retryT_, easeTMax));
		}
	}
	else {
		retry_->SetTexture("clear/UI1x1.png");
		if (retryT_ > 0.0f) {
			retryT_ -= deltaTime;
		}
		else {
			retryT_ = 0.0f;
		}
		retry_->SetScale(EaseInOutSine<Vector3>(startScale, retry_->GetTransform().scale_, retryT_, easeTMax)); // 選択されていない場合は縮小
	}

	if (currentItem_ == 2) {
		decisionEmitter_->SetPositionY(-8.4f);
		//decisionEmitter_->SetScale({ 1.4f,0.2f,0.0f });
		backSelect_->SetTexture("clear/UI2_1x1.png");
		if (!isDecision_) {
			selectT_ += deltaTime;
			if (selectT_ > easeTMax) {
				selectT_ -= easeTMax; // ループさせるために初期化
			}
			backSelect_->SetScale(EaseInOutSine<Vector3>(startScale, endScale, selectT_, easeTMax));
		}
		else {
			selectT_ += deltaTime;
			if (selectT_ > easeTMax) {
				selectT_ = easeTMax;
			}
			backSelect_->SetScale(EaseInSine<Vector3>(backSelect_->GetTransform().scale_, endScale, selectT_, easeTMax));
		}
	}
	else {
		backSelect_->SetTexture("clear/UI1x1.png");
		if (selectT_ > 0.0f) {
			selectT_ -= deltaTime;
		}
		else {
			selectT_ = 0.0f;
		}
		backSelect_->SetScale(EaseInOutSine<Vector3>(startScale, backSelect_->GetTransform().scale_, selectT_, easeTMax)); // 選択されていない場合は縮小
	}
}
