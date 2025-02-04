#include "Framework.h"
#include"ImGuiManager.h"
#include <D3DResourceLeakChecker.h>
#include"myEngine/Frame/Frame.h"


void Framework::Run()
{
	// ゲームの初期化
	Initialize();

	while (true) // ゲームループ
	{
		// 更新
		Update();
		// 終了リクエストが来たら抜ける
		if (IsEndRequest()) {
			break;
		}
		// 描画
		Draw();
	}
	// ゲームの終了
	Finalize();
}

void Framework::Initialize()
{

	D3DResourceLeakChecker();

	///---------WinApp--------
	// WindowsAPIの初期化
	winApp = WinApp::GetInstance();
	winApp->Initialize();
	///-----------------------

	///---------DirectXCommon----------
	// DirectXCommonの初期化
	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(winApp);
	///--------------------------------

	/// ---------ImGui---------
#ifdef _DEBUG
	ImGuiManager::GetInstance()->Initialize(winApp);
#endif // _DEBUG
	/// -----------------------

	///--------SRVManager--------
	// SRVマネージャの初期化
	srvManager = SrvManager::GetInstance();
	srvManager->Initialize();
	///--------------------------

	// offscreenのSRV作成
	dxCommon->CreateOffscreenSRV();
	// depthのSRV作成
	dxCommon->CreateDepthSRV();

	///----------Input-----------
	// 入力の初期化
	input = Input::GetInstance();
	input->Init(winApp->GetHInstance(), winApp->GetHwnd());
	///--------------------------

	///-----------TextureManager----------
	textureManager_ = TextureManager::GetInstance();
	textureManager_->Initialize(srvManager);
	///-----------------------------------

	///-----------ModelManager------------
	modelManager_ = ModelManager::GetInstance();
	modelManager_->Initialize(srvManager);
	///----------------------------------

	///----------SpriteCommon------------
	// スプライト共通部の初期化
	spriteCommon = SpriteCommon::GetInstance();
	spriteCommon->Initialize();
	///----------------------------------

	///----------Object3dCommon-----------
	// 3Dオブジェクト共通部の初期化
	object3dCommon = Object3dCommon::GetInstance();
	object3dCommon->Initialize();
	///-----------------------------------

	///----------ParticleCommon------------
	particleCommon = ParticleCommon::GetInstance();
	particleCommon->Initialize(dxCommon);
	///------------------------------------

	///---------Audio-------------
	audio = Audio::GetInstance();
	audio->Initialize();
	///---------------------------

	///-------CollisionManager--------------
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();
	///-------------------------------------

	///-------SceneManager--------
	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->Initialize();
	///---------------------------

	///-------OffScreen--------
	offscreen_ = std::make_unique<OffScreen>();
	offscreen_->Initialize();
	///------------------------
	
	///-------DrawLine3D-------
	line3d_ = DrawLine3D::GetInstance();
	line3d_->Initialize();
	///------------------------

	LightGroup::GetInstance()->Initialize();
	
	/// 時間の初期化
	Frame::Init();

}

void Framework::Finalize()
{
	sceneManager_->Finalize();

	// WindowsAPIの終了処理
	winApp->Finalize();

	/// -------TextureManager-------
	textureManager_->Finalize();
	///-----------------------------

	/// -------ModelCommon-------
	modelManager_->Finalize();
	///---------------------------

#ifdef _DEBUG
	ImGuiManager::GetInstance()->Finalize();
#endif // _DEBUG
	line3d_->Finalize();
	srvManager->Finalize();
	audio->Finalize();
	LightGroup::GetInstance()->Finalize();
	object3dCommon->Finalize();
	spriteCommon->Finalize();
	particleCommon->Finalize();
	dxCommon->Finalize();
	delete sceneFactory_;
}

void Framework::Update()
{
	/// deltaTimeの更新
	Frame::Update();
#ifdef _DEBUG
	ImGuiManager::GetInstance()->Begin();
#endif // _DEBUG
	offscreen_->DrawCommonSetting();
	sceneManager_->Update();
	collisionManager_->Update();
#ifdef _DEBUG
	DisplayFPS();
	ImGuiManager::GetInstance()->End();
#endif // _DEBUG

	/// -------更新処理開始----------

	// -------Input-------
	// 入力の更新
	input->Update();
	// -------------------

	/// -------更新処理終了----------
	endRequest_ = winApp->ProcessMessage();
}

void Framework::LoadResource()
{
	// モデル読み込み
	modelManager_->LoadModel("debug/cube.obj");

	modelManager_->LoadModel("game/Player.obj"); // プレイヤー
	modelManager_->LoadModel("game/playerSwitchBlockBlack.obj");
	modelManager_->LoadModel("game/playerSwitchBlockWhite.obj");
	modelManager_->LoadModel("clear/0.obj");
	modelManager_->LoadModel("clear/1.obj");
	modelManager_->LoadModel("clear/2.obj");
	modelManager_->LoadModel("clear/3.obj");
	modelManager_->LoadModel("clear/4.obj");
	modelManager_->LoadModel("clear/5.obj");
	modelManager_->LoadModel("clear/6.obj");
	modelManager_->LoadModel("clear/7.obj");
	modelManager_->LoadModel("clear/8.obj");
	modelManager_->LoadModel("clear/9.obj");
	modelManager_->LoadModel("clear/stage.obj");
	modelManager_->LoadModel("clear/Retry.obj");
	modelManager_->LoadModel("clear/nextStage.obj");
	modelManager_->LoadModel("clear/backSelect.obj");
	modelManager_->LoadModel("game/A.obj");
	modelManager_->LoadModel("game/blackBlock.obj");
	modelManager_->LoadModel("game/goal.obj");
	modelManager_->LoadModel("game/gravityBlockDown.obj");
	modelManager_->LoadModel("game/gravityBlockUp.obj");
	modelManager_->LoadModel("game/L.obj");
	modelManager_->LoadModel("game/notouchBlackBlock.obj");
	modelManager_->LoadModel("game/notouchWhiteBlock.obj");
	modelManager_->LoadModel("game/openBook.obj");
	modelManager_->LoadModel("game/Player.obj");
	modelManager_->LoadModel("game/R.obj");
	modelManager_->LoadModel("game/UI.obj");
	modelManager_->LoadModel("game/whiteBlock.obj");
	modelManager_->LoadModel("game/whiteBlock.obj");
	modelManager_->LoadModel("select/book.obj");
	modelManager_->LoadModel("title/title.obj");
	modelManager_->LoadModel("title/titleStart.obj");
	
	// テクスチャ読み込み
	textureManager_->LoadTexture("debug/uvChecker.png");

	textureManager_->LoadTexture("game/playerWhite.png"); // プレイヤー白状態
	textureManager_->LoadTexture("game/playerBlack.png"); // プレイヤー黒状態
	textureManager_->LoadTexture("clear/UI1x1.png"); // プレイヤー色反転ブロック
	textureManager_->LoadTexture("clear/UI2_1x1.png");
	textureManager_->LoadTexture("game/A.png");
	textureManager_->LoadTexture("game/block.png");
	textureManager_->LoadTexture("game/book.png");
	textureManager_->LoadTexture("game/goal.png");
	textureManager_->LoadTexture("game/gravityBlockDown.png");
	textureManager_->LoadTexture("game/gravityBlockUp.png");
	textureManager_->LoadTexture("game/L.png");
	textureManager_->LoadTexture("game/noTouchBlackBlock.png");
	textureManager_->LoadTexture("game/noTouchWhiteBlock.png");
	textureManager_->LoadTexture("game/openBook.png");
	textureManager_->LoadTexture("game/player.png");
	textureManager_->LoadTexture("game/playerFlame.png");
	textureManager_->LoadTexture("game/playerBlack.png");
	textureManager_->LoadTexture("game/playerWhite.png");
	textureManager_->LoadTexture("game/R.png");
	textureManager_->LoadTexture("game/ui.png");
	textureManager_->LoadTexture("menu/0.png");
	textureManager_->LoadTexture("menu/1.png");
	textureManager_->LoadTexture("menu/2.png");
	textureManager_->LoadTexture("menu/3.png");
	textureManager_->LoadTexture("menu/4.png");
	textureManager_->LoadTexture("menu/5.png");
	textureManager_->LoadTexture("menu/6.png");
	textureManager_->LoadTexture("menu/7.png");
	textureManager_->LoadTexture("menu/8.png");
	textureManager_->LoadTexture("menu/9.png");
	textureManager_->LoadTexture("menu/backGame.png");
	textureManager_->LoadTexture("menu/backGround.png");
	textureManager_->LoadTexture("menu/backSelect.png");
	textureManager_->LoadTexture("menu/pointer.png");
	textureManager_->LoadTexture("menu/Restart.png");
	textureManager_->LoadTexture("menu/stage.png");
	textureManager_->LoadTexture("particle/Arrow.png");
	textureManager_->LoadTexture("particle/blackBlock1x1.png");
	textureManager_->LoadTexture("particle/whiteBlock1x1.png");
	textureManager_->LoadTexture("select/book.png");
	textureManager_->LoadTexture("title/title.png");
	textureManager_->LoadTexture("game/playerSwitchBlockBlack.png");
	textureManager_->LoadTexture("game/playerSwitchBlockWhite.png");
}

void  Framework::PlaySounds() 
{
	
}

void Framework::Draw()
{

}

void Framework::DisplayFPS()
{
#ifdef _DEBUG
	ImGuiIO& io = ImGui::GetIO();

	// FPSを取得
	float fps = Frame::GetFPS();  // FPSの取得

	// FPSを表示するウィンドウを固定位置に設定
	ImGui::SetNextWindowPos(ImVec2(1230, 0), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f); // 背景を透明に設定

	// ウィンドウフラグを設定
	ImGui::Begin("FPS Overlay", nullptr,
		ImGuiWindowFlags_NoTitleBar |         // タイトルバーを非表示
		ImGuiWindowFlags_NoResize |          // リサイズを禁止
		ImGuiWindowFlags_NoMove |            // ウィンドウの移動を禁止
		ImGuiWindowFlags_NoScrollbar |       // スクロールバーを非表示
		ImGuiWindowFlags_NoCollapse |        // 折りたたみボタンを非表示
		ImGuiWindowFlags_AlwaysAutoResize |  // 自動サイズ調整
		ImGuiWindowFlags_NoBackground        // 背景を非表示
	);

	// FPSを緑色で表示
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 255, 50, 255));  // 緑色に設定
	ImGui::Text("%.1f", fps);  // FPSを表示
	ImGui::PopStyleColor();

	ImGui::End();

#endif // _DEBUG
}
