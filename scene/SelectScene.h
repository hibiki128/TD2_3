#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "DebugCamera.h"
#include "Input.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "SpriteCommon.h"
#include "application/Scene/SelectScene/MapPrev.h"
#include "application/Scene/SelectScene/SelectUI.h"

class SelectScene : public BaseScene {
  public: // メンバ関数
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;

    /// <summary>
    /// オフスクリーン上に描画
    /// </summary>
    void DrawForOffScreen() override;

    ViewProjection *GetViewProjection() override { return &vp_; }

  private:
    void Debug();

    void CameraUpdate();

    void ChangeScene();

    void MapLoad();

    void MapSelect();

    void CameraMove();

    void SetStage();

    void DeleteData();

  private:
    Audio *audio_;
    Input *input_;
    Object3dCommon *objCommon_;
    SpriteCommon *spCommon_;
    ParticleCommon *ptCommon_;

    ViewProjection vp_;
    std::unique_ptr<DebugCamera> debugCamera_;

    // マップチップフィールド
    std::vector<std::unique_ptr<MapPrev>> mapPrevs_;
    std::unique_ptr<SelectUI> selectUI_;

    int stageNum = 17;
    int currentStage = 0;
    float cameraT_ = 0.0f;
    bool isMoveCamera_ = false;
    float startPos = 0.0f;
    float endPos = 0.0f;

    // 音関連
    uint32_t BGM_;
    uint32_t selectSE_;
    uint32_t desitionSE_;

    std::string filePath;
    bool BackGameScene_ = false;
    bool anyDecisionMade = false;

    // 背景
    std::unique_ptr<Sprite> spriteBackGround_;
};
