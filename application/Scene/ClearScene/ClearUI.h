#pragma once
#include "application/Base/BaseObject.h"
#include <ParticleEmitter.h>
class Input;
class ClearUI {
  public:
    void Init(bool isPlayerWhite);
    void Update();
    void Draw(const ViewProjection &vp);
    void DrawParticle(const ViewProjection &vp);
    void DrawTexts(const ViewProjection &vp);
    void Debug();
    int GetItemNum() { return currentItem_; }
    void SetStageNum(const int &stageNum) { stageNum_ = stageNum; }
    void SetDecision(const bool &dicision) { isDecision_ = dicision; }
    void SetCoin(int coinNum) { coinNum_ = coinNum; }

  private:
    void MenuOperation();
    void InitNumbers();
    void MoveUI();
    void CoinUpdate();

  private:
    Input *input_ = nullptr;
    std::unique_ptr<BaseObject> book_;
    std::unique_ptr<BaseObject> stage_;
    std::unique_ptr<BaseObject> nextStage_;
    std::unique_ptr<BaseObject> backSelect_;
    std::unique_ptr<BaseObject> retry_;
    std::unique_ptr<BaseObject> singleDigit_;
    std::unique_ptr<BaseObject> twoDigit_;
    std::unique_ptr<BaseObject> animaChara_;
    std::unique_ptr<BaseObject> animaGoal_;
    std::unique_ptr<ParticleEmitter> decisionEmitter_;

    std::vector<std::unique_ptr<BaseObject>> coins_;

    int currentItem_;
    int coinNum_;
    int stageNum_;

    float coolTime_;
    float nextT_;
    float retryT_;
    float selectT_;

    bool isDecision_ = false;
    bool isPlayedSE_[3] = {false};

    float t_[3];                  // アニメーションの進行度
    float tMax_ = 0.75f;          // アニメーションの最大時間
    float deltaTime_ = 0.016f;    // フレームごとの進行時間（仮）
    size_t currentCoinIndex_ = 0; // 現在アニメーション中のコイン
    float initialY_ = -5.7f;      // コインの初期Y座標
    float timer_ = 1.0f;

    uint32_t coinSE_;
    uint32_t desitionSE_;
    uint32_t selectSE_;
};
