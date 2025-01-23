#pragma once
#include <string>
#include "WorldTransform.h"  // Transformの定義があるヘッダーをインクルード
#include "ParticleManager.h" // ParticleManagerのインクルード
#include "ViewProjection.h"
#ifdef _DEBUG
#include "imgui.h" // ImGuiのインクルード
#endif // _DEBUG

#include "externals/nlohmann/json.hpp"

#include <fstream>
#include <filesystem>


class ParticleEmitter {
public:
    // コンストラクタでメンバ変数を初期化
    ParticleEmitter();

    void Initialize(const std::string& name, const std::string& fileName);

    // 更新処理を行うUpdate関数
    void Update();

    void UpdateOnce();

    void Draw(const ViewProjection& vp_);

    void DrawEmitter();

    void imgui(); // ImGui用の関数を追加

    void SetPosition(const Vector3& position) { transform_.translation_ = position; }
    void SetScale(const Vector3& scale) { transform_.scale_ = scale; }
    void SetCount(const int& count) { count_ = count; }
    void SetActive(bool isActive) { isActive_ = isActive; }
    void SetStartRotate(const Vector3& startRotate) { startRote_ = startRotate; }
    void SetEndRotate(const Vector3& endRotate) { endRote_ = endRotate; }
    void SetFrequency(const float& frequency) { emitFrequency_ = frequency; }

private:
    // パーティクルを発生させるEmit関数
    void Emit();
    void SaveToJson();
    void LoadFromJson();
private:
    using json = nlohmann::json;
    float elapsedTime_;         // 経過時間

    std::string name_;          // パーティクルの名前
    WorldTransform transform_;       // 位置や回転などのトランスフォーム
    int count_;                 // 一度に発生させるパーティクルの数

    float emitFrequency_;       // パーティクルの発生頻度
    float lifeTimeMin_;         // ライフタイムの最小値
    float lifeTimeMax_;         // ライフタイムの最大値
    float alphaMin_;
    float alphaMax_;
    float scaleMin_;
    float scaleMax_;

    Vector3 velocityMin_;       // 速度の最小値
    Vector3 velocityMax_;       // 速度の最大値
    Vector3 startScale_;
    Vector3 endScale_;
    Vector3 startAcce_;
    Vector3 endAcce_;
    Vector3 startRote_ = {};
    Vector3 endRote_ = {};
    Vector3 rotateVelocityMin_;
    Vector3 rotateVelocityMax_;
    Vector3 allScaleMin_;
    Vector3 allScaleMax_;

    bool isRandomScale_ = false;
    bool isAllRamdomScale_ = false;
    bool isRandomColor_ = true;
    bool isRandomRotate_ = false;
    bool isVisible_;
    bool isBillBoard_ = true;
    bool isActive_ = false;
    bool isAcceMultiply_ = false;
    bool isSinMove_ = false;
    bool isFaceDirection_ = false;

    std::unique_ptr<ParticleManager> Manager_;
};