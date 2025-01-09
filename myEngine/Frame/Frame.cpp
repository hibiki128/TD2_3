#include "Frame.h"
#include <chrono>

/// <summary>
/// 静的メンバ変数の定義
/// </summary>
std::chrono::high_resolution_clock::time_point Frame::lastTime_ = std::chrono::high_resolution_clock::now();
float Frame::deltaTime_ = 0.0f;
int Frame::frameCount_ = 0;
std::chrono::high_resolution_clock::time_point Frame::fpsTime_ = std::chrono::high_resolution_clock::now();
float Frame::fps_ = 0.0f;

/// <summary>
/// フレームの初期化処理
/// </summary>
void Frame::Init() {
    lastTime_ = std::chrono::high_resolution_clock::now();
    fpsTime_ = std::chrono::high_resolution_clock::now();
    frameCount_ = 0;
    fps_ = 0.0f;
}

/// <summary>
/// フレームの更新処理
/// </summary>
void Frame::Update() {
    // 現在の時刻を取得
    auto currentTime = std::chrono::high_resolution_clock::now();

    // 経過時間を計算
    std::chrono::duration<float> elapsed = currentTime - lastTime_;
    deltaTime_ = elapsed.count(); // 秒単位の経過時間

    // フレームカウントをインクリメント
    frameCount_++;

    // 1秒経過したかどうかをチェック
    std::chrono::duration<float> fpsElapsed = currentTime - fpsTime_;
    if (fpsElapsed.count() >= 1.0f) {
        // FPSを計算
        fps_ = static_cast<float>(frameCount_) / fpsElapsed.count(); // フレーム数を経過時間で割る
        frameCount_ = 0;  // フレームカウントをリセット
        fpsTime_ = currentTime;  // 次の1秒間の開始時間を記録
    }

    // 次回の更新のために現在の時刻を記録
    lastTime_ = currentTime;
}

/// <summary>
/// 前回の更新からの経過時間を取得
/// </summary>
/// <returns>前回の更新からの経過時間</returns>
float Frame::DeltaTime() {
    return deltaTime_; // 経過時間を返す
}

/// <summary>
/// 現在のFPSを取得
/// </summary>
/// <returns>現在のFPS</returns>
float Frame::GetFPS() {
    return fps_; // 現在のFPSを返す
}
