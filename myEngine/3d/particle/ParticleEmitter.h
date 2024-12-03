#pragma once
#include <string>
#include "WorldTransform.h"  // Transformの定義があるヘッダーをインクルード
#include "ParticleManager.h" // ParticleManagerのインクルード
#include "ViewProjection.h"
#ifdef _DEBUG
#include "imgui.h" // ImGuiのインクルード
#endif // _DEBUG
#include "Object3d.h"
#include"GlobalVariables.h"

class ParticleEmitter {
public:
    // コンストラクタでメンバ変数を初期化
    ParticleEmitter();

    void Initialize(const std::string& name, const std::string& fileName);

    // 更新処理を行うUpdate関数
    void Update(const ViewProjection& vp_);

    void UpdateOnce(const ViewProjection& vp_);

    void Draw();

    void DrawEmitter(const ViewProjection& vp_);

    void RenderImGui(); // ImGui用の関数を追加

    void SetPosition(const Vector3& position) { transform_.translation_ = position; }
    void SetScale(const Vector3& scale) { transform_.scale_ = scale; }
    void SetCount(const int& count) { count_ = count; }
    void SetActive(bool isActive) { isActive_ = isActive; }
    void SetValue();

private:
    // パーティクルを発生させるEmit関数
    void Emit();

    void ApplyGlobalVariables();
    void AddItem();

    std::string name_;          // パーティクルの名前
    WorldTransform transform_;       // 位置や回転などのトランスフォーム
    int count_;                 // 一度に発生させるパーティクルの数

    float emitFrequency_;       // パーティクルの発生頻度
    float elapsedTime_;         // 経過時間
    // Emitに関連するメンバ変数を追加
    float lifeTimeMin_;         // ライフタイムの最小値
    float lifeTimeMax_;         // ライフタイムの最大値
    float alphaMin_;
    float alphaMax_;
    float deltaTime = 1.0f / 60.0f;
    float scaleMin;
    float scaleMax;

    Vector3 velocityMin_;       // 速度の最小値
    Vector3 velocityMax_;       // 速度の最大値
    Vector3 startScale_;
    Vector3 endScale_;
    Vector3 startAcce_;
    Vector3 endAcce_;
    Vector3 startRote_ = {};
    Vector3 endRote_ = {};
    Vector3 rotateVelocityMin;
    Vector3 rotateVelocityMax;
    Vector3 allScaleMin;
    Vector3 allScaleMax;

    bool isRandomScale = false;
    bool isAllRamdomScale = false;
    bool isRandomColor = true;
    bool isRandomRotate = false;
    bool isVisible;
    bool isBillBoard = true;
    bool isActive_ = false;
    bool isAcceMultiply = false;

    std::unique_ptr<Object3d> emitterObj;
    std::unique_ptr<ParticleManager> Manager_;

    GlobalVariables* globalVariables = nullptr;
    const char* groupName = nullptr;

};