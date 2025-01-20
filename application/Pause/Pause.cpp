#include "Pause.h"
#include"myEngine/Frame/Frame.h"
#include"SpriteCommon.h"
#include"math/Easing.h"

void Pause::Init()
{
	// インスタンス生成
	input_ = Input::GetInstance();
	backGround_ = std::make_unique<Sprite>();
	backGround_->Initialize("menu/backGround.png", spritePosition_, color_);

	// 各要素初期化
	alpha_E.start_.x = 0.0f;
	alpha_E.end_.x = 0.0f;
	alpha_E.T_ = 0.0f;
	EscapeCoolTime_ = 0.0f;

	// 各フラグ初期化
	isPause_ = false;

	InitText();

	pointer_E.start_ = { -360.0f,420.0f };
	pointer_E.end_ = { -360.0f,420.0f };
	stage_E.start_.x = -360.0f;
	stage_E.end_.x = -360.0f;
	backGame_E.start_.x = -360.0f;
	backGame_E.end_.x = -360.0f;
	backSelect_E.start_.x = -360.0f;
	backSelect_E.end_.x = -360.0f;
	restart_E.start_.x = -360.0f;
	restart_E.end_.x = -360.0f;
}

void Pause::Update()
{
	// メニューを開く
	OpenMenu();

	// メニュー操作
	MenuOperation();

	// 文字の演出
	MoveText();

	// オブジェクト更新
	UpdateText();

	backGround_->SetSize({ 1280.0f,720.0f });
	backGround_->SetAlpha(color_.w);
	Debug();
}

void Pause::Draw(const ViewProjection& vp)
{

	SpriteCommon::GetInstance()->DrawCommonSetting();
	backGround_->Draw();
	Pointer_->Draw();
	Stage_->Draw();
	backGame_->Draw();
	backSelect_->Draw();
	Restart_->Draw();

}
#pragma region 文字関連

void Pause::InitText()
{
	// インスタンス生成
	Stage_ = std::make_unique<Sprite>();
	backGame_ = std::make_unique<Sprite>();
	backSelect_ = std::make_unique<Sprite>();
	Restart_ = std::make_unique<Sprite>();
	Pointer_ = std::make_unique<Sprite>();

	// 初期化
	Stage_->Initialize("menu/stage.png", backGamePos_, { 1,1,1,1 }, { 0.5f,0.5f });
	backGame_->Initialize("menu/backGame.png", backGamePos_, { 1,1,1,1 }, { 0.5f,0.5f });
	backSelect_->Initialize("menu/backSelect.png", backSelectPos_, { 1,1,1,1 }, { 0.5f,0.5f });
	Restart_->Initialize("menu/Restart.png", restartPos_, { 1,1,1,1 }, { 0.5f,0.5f });
	Pointer_->Initialize("menu/pointer.png", pointerPos_, { 1,1,1,1 }, { 0.5f,0.5f });

	// 初期位置(右から中央へやるため画面外)
	stagePos_ = { -360.0f,100.0f };
	backGamePos_ = { -360.0f,420.0f };
	backSelectPos_ = { -360.0f ,600.0f };
	restartPos_ = { -360.0f ,510.0f };
	pointerPos_ = { -360.0f,420.0f };

	stageSize_ = Stage_->GetSize();
	backGameSize_ = backGame_->GetSize();
	backSelectSize_ = backSelect_->GetSize();
	restartSize_ = Restart_->GetSize();
	pointerSize_ = Pointer_->GetSize();
}

void Pause::UpdateText()
{
	// 各種アニメーションパラメータを設定
	backGame_E.TMax_ = 0.5f;
	backSelect_E.TMax_ = 0.5f;
	restart_E.TMax_ = 0.5f;
	stage_E.TMax_ = 0.5f;
	pointer_E.TMax_ = 0.5f;

	// ポーズが解除された場合の初期設定
	if (!isPause_ && previousIsPause_) {
		pointer_E.start_.x = 500.0f;
		pointer_E.end_.x = 1640.0f;
		pointer_E.T_ = 0.0f;
		stage_E.start_.x = 600.0f;
		stage_E.end_.x = 1640.0f;
		stage_E.T_ = 0.0f;
		backGame_E.start_.x = 640.0f;
		backGame_E.end_.x = 1640.0f; // 右側に移動する終点位置
		backGame_E.T_ = 0.0f;
		backSelect_E.start_.x = 640.0f;
		backSelect_E.end_.x = 1640.0f;
		backSelect_E.T_ = 0.0f;
		restart_E.start_.x = 640.0f;
		restart_E.end_.x = 1640.0f;
		restart_E.T_ = 0.0f;
	}
	// ポーズがかかった場合の初期設定
	else if (isPause_ && !previousIsPause_) {
		pointer_E.start_.x = -360.0f;
		pointer_E.end_.x = 500.0f;
		pointer_E.T_ = 0.0f;
		stage_E.start_.x = -360.0f;
		stage_E.end_.x = 600.0f;
		stage_E.T_ = 0.0f;
		backGame_E.start_.x = -360.0f;
		backGame_E.end_.x = 640.0f;
		backGame_E.T_ = 0.0f;
		backSelect_E.start_.x = -360.0f;
		backSelect_E.end_.x = 640.0f;
		backSelect_E.T_ = 0.0f;
		restart_E.start_.x = -360.0f;
		restart_E.end_.x = 640.0f;
		restart_E.T_ = 0.0f;
	}

	// イージングによるアニメーション更新
	stage_E.T_ += Frame::DeltaTime();
	if (stage_E.T_ >= 0.1f) {
		pointer_E.T_ += Frame::DeltaTime();
		backGame_E.T_ += Frame::DeltaTime();
	}
	if (restart_E.T_ >= 0.1f) {
		backSelect_E.T_ += Frame::DeltaTime();
	}
	if (backGame_E.T_ >= 0.1f) {
		restart_E.T_ += Frame::DeltaTime();
	}

	// 時間の制限を適用
	if (pointer_E.T_ >= pointer_E.TMax_) {
		pointer_E.T_ = pointer_E.TMax_;
	}
	if (stage_E.T_ >= stage_E.TMax_) {
		stage_E.T_ = stage_E.TMax_;
	}
	if (backGame_E.T_ >= backGame_E.TMax_) {
		backGame_E.T_ = backGame_E.TMax_;
	}
	if (backSelect_E.T_ >= backSelect_E.TMax_) {
		backSelect_E.T_ = backSelect_E.TMax_;
	}
	if (restart_E.T_ >= restart_E.TMax_) {
		restart_E.T_ = restart_E.TMax_;
	}

	// イージング関数を使って位置を更新
	pointerPos_.x = EaseInOutBack<float>(pointer_E.start_.x, pointer_E.end_.x, pointer_E.T_, pointer_E.TMax_);
	stagePos_.x = EaseInOutBack<float>(stage_E.start_.x, stage_E.end_.x, stage_E.T_, stage_E.TMax_);
	backGamePos_.x = EaseInOutBack<float>(backGame_E.start_.x, backGame_E.end_.x, backGame_E.T_, backGame_E.TMax_);
	backSelectPos_.x = EaseInOutBack<float>(backSelect_E.start_.x, backSelect_E.end_.x, backSelect_E.T_, backSelect_E.TMax_);
	restartPos_.x = EaseInOutBack<float>(restart_E.start_.x, restart_E.end_.x, restart_E.T_, restart_E.TMax_);

	// すべてのテキストが左に移動しきったかをチェック
	if (backGamePos_.x >= 1640.0f && backSelectPos_.x >= 1640.0f && restartPos_.x >= 1640.0f) {
		textMovedRight_ = true; // フラグを設定
		stagePos_.x = -360.0f;
		backGamePos_.x = -360.0f;
		backSelectPos_.x = -360.0f;
		restartPos_.x = -360.0f;
		stage_E.start_.x = -360.0f;
		stage_E.end_.x = -360.0f;
		backGame_E.start_.x = -360.0f;
		backGame_E.end_.x = -360.0f;
		backSelect_E.start_.x = -360.0f;
		backSelect_E.end_.x = -360.0f;
		restart_E.start_.x = -360.0f;
		restart_E.end_.x = -360.0f;
	}
	else {
		textMovedRight_ = false; // フラグをリセット
	}

	// 現在のポーズ状態を前フレームの状態として保存
	previousIsPause_ = isPause_;
}


void Pause::MoveText()
{
	Pointer_->SetPosition(pointerPos_);
	Stage_->SetPosition(stagePos_);
	backGame_->SetPosition(backGamePos_);
	backSelect_->SetPosition(backSelectPos_);
	Restart_->SetPosition(restartPos_);
	Stage_->SetSize(stageSize_ / 1.25f);
	backGame_->SetSize(backGameSize_ / 1.75f);
	backSelect_->SetSize(backSelectSize_ / 1.75f);
	Restart_->SetSize(restartSize_ / 1.75f);
	Pointer_->SetSize(pointerSize_ / 1.75f);
}
#pragma endregion

void Pause::OpenMenu()
{
	alpha_E.TMax_ = 0.2f;

	if (input_->TriggerKey(DIK_ESCAPE) && !isPause_ && CanEscape_) {
		alpha_E.start_.x = 0.0f;
		alpha_E.end_.x = 0.99f;
		alpha_E.T_ = 0.0f;
		isPause_ = true;
		EscapeCoolTime_ = 0.7f;
	}

	if (EscapeCoolTime_ > 0.0f) {
		EscapeCoolTime_ -= Frame::DeltaTime();
		CanEscape_ = false;
	}
	else {
		CanEscape_ = true;
	}

	if (input_->TriggerKey(DIK_ESCAPE) && isPause_ && CanEscape_) {
		isPause_ = false;
		EscapeCoolTime_ = 0.7f;
	}
	if (textMovedRight_) {
		alpha_E.start_.x = 0.99f;
		alpha_E.end_.x = 0.0f;
		alpha_E.T_ = 0.0f;
		textMovedRight_ = false;
	}

	alpha_E.T_ += Frame::DeltaTime();
	if (alpha_E.T_ >= alpha_E.TMax_) {
		alpha_E.T_ = alpha_E.TMax_;
	}

	color_.w = EaseInSine<float>(alpha_E.start_.x, alpha_E.end_.x, alpha_E.T_, alpha_E.TMax_);
}

void Pause::MenuOperation()
{
	
	const float easeTMax = 0.2f;
	if (currentItem_ < -2) {
		currentItem_ = 0;
	}
	if (currentItem_ > 0) {
		currentItem_ = -2;
	}

	if (isPause_) {
		if (input_->TriggerKey(DIK_W)) {
			currentItem_++;
			pointer_E.start_.y = pointerPos_.y;
			pointerYT_ = 0.0f;
		}
		if (input_->TriggerKey(DIK_S)) {
			currentItem_--;
			pointer_E.start_.y = pointerPos_.y;
			pointerYT_ = 0.0f;
		}
	}
	else {
		currentItem_ = 0;
	}

	if (currentItem_ == 0) {
		pointer_E.end_.y = 420.0f;
	}
	else if (currentItem_ == -1) {
		pointer_E.end_.y = 510.0f;
	}
	else if (currentItem_ == -2) {
		pointer_E.end_.y = 600.0f;
	}

	pointerYT_ += Frame::DeltaTime();
	if (pointerYT_ >= easeTMax) {
		pointerYT_ = easeTMax;
	}

	if (currentItem_ == 0 && input_->TriggerKey(DIK_SPACE)) {
		isPause_ = false;
	}

	pointerPos_.y = EaseInSine<float>(pointer_E.start_.y, pointer_E.end_.y, pointerYT_, easeTMax);

}

void Pause::Debug()
{
	//ImGui::Begin("Pause");
	//ImGui::DragFloat2("ゲームに戻る", &backGamePos.x, 0.1f);
	//ImGui::DragFloat2("セレクトに戻る", &backSelectPos.x, 0.1f);
	//ImGui::DragFloat2("リスタート", &restartPos.x, 0.1f);
	//ImGui::DragFloat2("ゲーム サイズ", &backGameSize.x, 0.1f);
	//ImGui::DragFloat2("セレクト サイズ", &backSelectSize.x, 0.1f);
	//ImGui::DragFloat2("リスタート サイズ", &restartSize.x, 0.1f);
	//ImGui::Text("現在のアイテム %d", currentItem_);
	//ImGui::End();
}
