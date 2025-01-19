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
	alpha_E.start_ = 0.0f;
	alpha_E.end_ = 0.0f;
	alpha_E.T_ = 0.0f;
	EscapeCoolTime_ = 0.0f;

	// 各フラグ初期化
	isPause_ = false;

	InitText();

	backGame_E.start_ = -360.0f;
	backGame_E.end_ = -360.0f;
	backSelect_E.start_ = -360.0f;
	backSelect_E.end_ = -360.0f;
	restart_E.start_ = -360.0f;
	restart_E.end_ = -360.0f;
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
	backGame_->Draw();
	backSelect_->Draw();
	Restart_->Draw();

}

void Pause::InitText()
{
	// インスタンス生成
	backGame_ = std::make_unique<Sprite>();
	backSelect_ = std::make_unique<Sprite>();
	Restart_ = std::make_unique<Sprite>();

	// 初期化
	backGame_->Initialize("menu/backGame.png", backGamePos, { 1,1,1,1 }, { 0.5f,0.5f });
	backSelect_->Initialize("menu/backSelect.png", backSelectPos, { 1,1,1,1 }, { 0.5f,0.5f });
	Restart_->Initialize("menu/Restart.png", restartPos, { 1,1,1,1 }, { 0.5f,0.5f });

	// 初期位置(右から中央へやるため画面外)
	backGamePos = { -360.0f,420.0f };
	backSelectPos = { -360.0f ,600.0f };
	restartPos = { -360.0f ,510.0f };

	backGameSize = backGame_->GetSize();
	backSelectSize = backSelect_->GetSize();
	restartSize = Restart_->GetSize();
}

void Pause::UpdateText()
{
	// 各種アニメーションパラメータを設定
	backGame_E.TMax_ = 0.5f;
	backSelect_E.TMax_ = 0.5f;
	restart_E.TMax_ = 0.5f;

	// ポーズが解除された場合の初期設定
	if (!isPause_ && previousIsPause_) {
		backGame_E.start_ = 640.0f;
		backGame_E.end_ = 1640.0f; // 右側に移動する終点位置
		backGame_E.T_ = 0.0f;
		backSelect_E.start_ = 640.0f;
		backSelect_E.end_ = 1640.0f;
		backSelect_E.T_ = 0.0f;
		restart_E.start_ = 640.0f;
		restart_E.end_ = 1640.0f;
		restart_E.T_ = 0.0f;
	}
	// ポーズがかかった場合の初期設定
	else if (isPause_ && !previousIsPause_) {
		backGame_E.start_ = -360.0f;
		backGame_E.end_ = 640.0f;
		backGame_E.T_ = 0.0f;
		backSelect_E.start_ = -360.0f;
		backSelect_E.end_ = 640.0f;
		backSelect_E.T_ = 0.0f;
		restart_E.start_ = -360.0f;
		restart_E.end_ = 640.0f;
		restart_E.T_ = 0.0f;
	}

	// イージングによるアニメーション更新
	backGame_E.T_ += Frame::DeltaTime();
	if (restart_E.T_ >= 0.1f) {
		backSelect_E.T_ += Frame::DeltaTime();
	}
	if (backGame_E.T_ >= 0.1f) {
		restart_E.T_ += Frame::DeltaTime();
	}

	// 時間の制限を適用
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
	backGamePos.x = EaseInOutBack<float>(backGame_E.start_, backGame_E.end_, backGame_E.T_, backGame_E.TMax_);
	backSelectPos.x = EaseInOutBack<float>(backSelect_E.start_, backSelect_E.end_, backSelect_E.T_, backSelect_E.TMax_);
	restartPos.x = EaseInOutBack<float>(restart_E.start_, restart_E.end_, restart_E.T_, restart_E.TMax_);

	// すべてのテキストが左に移動しきったかをチェック
	if (backGamePos.x >= 1640.0f && backSelectPos.x >= 1640.0f && restartPos.x >= 1640.0f) {
		textMovedRight_ = true; // フラグを設定
	}
	else {
		textMovedRight_ = false; // フラグをリセット
	}

	// 現在のポーズ状態を前フレームの状態として保存
	previousIsPause_ = isPause_;
}


void Pause::MoveText()
{
	backGame_->SetPosition(backGamePos);
	backSelect_->SetPosition(backSelectPos);
	Restart_->SetPosition(restartPos);
	backGame_->SetSize(backGameSize / 1.75f);
	backSelect_->SetSize(backSelectSize / 1.75f);
	Restart_->SetSize(restartSize / 1.75f);
}

void Pause::OpenMenu()
{
	alpha_E.TMax_ = 0.1f;

	if (input_->TriggerKey(DIK_ESCAPE) && !isPause_ && CanEscape_) {
		alpha_E.start_ = 0.0f;
		alpha_E.end_ = 0.95f;
		alpha_E.T_ = 0.0f;
		isPause_ = true;
		EscapeCoolTime_ = 0.2f;
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
		alpha_E.start_ = 0.95f;
		alpha_E.end_ = 0.0f;
		alpha_E.T_ = 0.0f;
		EscapeCoolTime_ = 0.2f;
	}

	alpha_E.T_ += Frame::DeltaTime();
	if (alpha_E.T_ >= alpha_E.TMax_) {
		alpha_E.T_ = alpha_E.TMax_;
	}

	color_.w = EaseInSine<float>(alpha_E.start_, alpha_E.end_, alpha_E.T_, alpha_E.TMax_);

}

void Pause::MenuOperation()
{
	if (isPause_) {
		if (input_->TriggerKey(DIK_UP)) {
			currentItem_++;
		}
		if (input_->TriggerKey(DIK_DOWN)) {
			currentItem_--;
		}
	}
}

void Pause::Debug()
{
	ImGui::Begin("PauseText");
	ImGui::DragFloat2("ゲームに戻る", &backGamePos.x, 0.1f);
	ImGui::DragFloat2("セレクトに戻る", &backSelectPos.x, 0.1f);
	ImGui::DragFloat2("リスタート", &restartPos.x, 0.1f);
	ImGui::DragFloat2("ゲーム サイズ", &backGameSize.x, 0.1f);
	ImGui::DragFloat2("セレクト サイズ", &backSelectSize.x, 0.1f);
	ImGui::DragFloat2("リスタート サイズ", &restartSize.x, 0.1f);
	ImGui::End();
}
