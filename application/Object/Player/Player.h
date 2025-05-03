#pragma once

// Engine
#include "Input.h"
#include "myEngine/utility/collider/Collider.h"

// Application
#include "application/Base/BaseObject.h"
#include "application/Object/MapChip/MapChipField.h"
#include "application/Transition/SquareTransition.h"

class Player : public BaseObject {
  public:
    // マップとの当たり判定情報
    struct CollisionMapInfo {
        bool hittingGround_ = false;
        bool hittingCeiling_ = false;
        bool hittingLeft_ = false;
        bool hittingRight_ = false;

        bool isOverlapping_ = false; // ブロックとの重なり状態

        Block *blockX = nullptr; // X方向で衝突したブロック
        Block *blockY = nullptr; // Y方向で衝突したブロック
    };

    void Init(const std::string className, int currentStageNum);
    void Update(MapChipField *mapChipField, bool title = false);
    void Draw(const ViewProjection &viewProjection, Vector3 offSet = {0.0f, 0.0f, 0.0f}) override;
    void DrawParticle(const ViewProjection &viewProjection);
    void DrawSprite(const ViewProjection &viewProjection, bool title = false);
    void DebugImGui() override;
    void Reset(bool title = false);
    void PlaySE();
    void BaseUpdate();

    // プレイヤーがゴールに到達しているか判定
    bool IsGoalReached(bool title = false);
    bool GetSquareTransition() {
        if (squareTransition_->GetCurrentStatus() == SquareTransition::Status::SquareIn) {
            return true;
        }
        return false;
    }
    // 現在の取得コイン数
    uint32_t GetCurrentCoinCount() { return currentCoinCount_; }
    bool GetFinishTransition() { return squareTransition_->IsFinished(); }

    // プレイヤーの位置を設定
    void SetInitialPosition(Vector3 playerInitialPosition) { this->transform_.translation_ = playerInitialPosition; }
    void SetTransitionStart() {
        if (squareTransition_->IsFinished()) {
            squareTransition_->Start(SquareTransition::Status::SquareIn, kResetTransitionTime);
        }
    }
    void SetClearAnima(bool title) {
        if (!title) {
            BaseObject::SetLoop(false);
            BaseObject::SetAnima("animation/playerGoal.gltf");
            BaseObject::SetRotationY(degreesToRadians(90.0f));
        }
    };
    bool CanJump() {
        if (!isJump_) {
            canJumpCooltime += 0.1f;
        } else {
            canJumpCooltime = 0.0f;
        }
        if (canJumpCooltime > 0.0f) {
            return true;
        }
        return false;
    }

    // プレイヤーの反転範囲の取得
    int GetInvertRangeX() { return xInvertRange_; }
    int GetInvertRangeY() { return yInvertRange_; }
    bool GetGoalAnimaFinish();

  private:
    #ifdef _DEBUG
    std::unique_ptr<BaseObject> dummyObject_;
    #endif

    const float kDeltaTime = 1.0f / 60.0f;

    // 入力
    Input *input_;

    ///
    /// 基本的なパラメータ
    ///

    // マップとの当たり判定情報
    CollisionMapInfo collisionMapInfo_;
    // 微小な値
    const float kBlank = 0.0001f;

    // サイズ
    //const float kWidth = 1.8f;
    //const float kHeight = 3.6f; // 縦長になるよう変更
    //const Vector3 kDefaultScale = {0.9f, 0.9f, 0.9f};

    const float kWidth = 1.35f;
    const float kHeight = 2.7f;
    const Vector3 kDefaultScale = {0.675f, 0.675f, 0.675f};

    // リセット時のトランジションにかける時間
    const float kResetTransitionTime = 0.3f;

    /*ブロック反転中、プレイヤーが動かないようにするために使用*/
    bool isInverting_ = false;          // ブロック反転中かどうか
    float invertTimer_ = 0.0f;          // タイマー
    const float invertDuration_ = 0.4f; // 反転アニメーションの合計時間

    // 重力反転状態かどうか
    bool isGravityReversed_ = false; // 初期状態は通常

    /*プレイヤーの現在の色*/
    enum class ColorState {
        White,
        Black,
    };
    // プレイヤーの色の状態
    ColorState colorState_ = ColorState::White;

    // 現在取得したコインの枚数
    uint32_t currentCoinCount_ = 0;

    //////////////////
    /*調整パラメーター*/
    /////////////////

    // 移動関連
    Vector3 velocity_;              // 速度
    const float kMoveSpeed = 0.15f; // 移動速度

    // ジャンプ関連
    float gravityAcceleration_; // 重力加速度
    float jumpAcceleration_;    // ジャンプ初速

    float offSetY_ = 0.0f; // 回転時の描画位置ずらすよう
    float inversT_ = 1.0f; // 反転時イージング用
    float startRote_ = 0.0f;
    float moveCoolTime_ = 0.0f; // 入力遅延用クールタイム

    // 反転可能範囲
    int xInvertRange_;
    int yInvertRange_;

    int prevDirection_ = 1; // 1: 右向き, -1: 左向き

    ///
    ///	その他
    ///

    // リセット時のトランジション
    std::unique_ptr<SquareTransition> squareTransition_;
    // プレイヤー反転範囲スプライト
    std::unique_ptr<Sprite> spritePlayerArea_;
    // ゴール接触時スプライト
    std::unique_ptr<Sprite> spriteGoalGuide_;
    std::unique_ptr<Sprite> spriteGoalGuideTitle_;
    // パーティクル
    ParticleEmitter *coinEmitter_ = nullptr;
    ParticleEmitter *runEmitter_ = nullptr;
    ParticleEmitter *changeEmitter_ = nullptr;

    // プレイヤー反転範囲スプライトのサイズ
    float xSpritePlayerAreaSize_ = 0.0f;
    float ySpritePlayerAreaSize_ = 0.0f;

    // ブロック反転のクールタイム
    float blockInvertCooldown_ = 0.0f;
    const float kBlockInvertCooldownTime = 0.7f; // 再使用までの時間

    // プレイヤー本体の色反転時の拡縮アニメーション
    bool isScaling_ = false; // スケーリング中かどうか
    bool isGoalAnimaFinish_ = false;
    float scaleTimer_;                 // アニメーションの進行度を管理
    float initialScale_ = 0.675f;        // アニメーション開始時スケール
    float targetScale_ = 0.3375f;        // 最小時スケール
    const float kScaleDuration = 0.5f; // アニメーションにかける時間
    bool isChangedColor_ = true;
    bool reached = false;
    void UpdateScalingAnimation();

    float prevRotY_; // プレイヤーの色反転ブロックを発動した際に現在の回転角を保存しておく

    // 現在選択されたステージ
    int currentStageNum_ = -1;
    void ChangeInvertRangeSizeForStageNum(); // 選択されたステージによって反転枠の大きさを変更する

    // 音関連
    uint32_t jumpSE_;
    uint32_t landingSE_;
    uint32_t walkSE_;
    uint32_t gravitySE_;
    uint32_t inversionSE_;
    uint32_t coinGetSE_;
    uint32_t invertDisabledSE_;
    float walkSEcoolTime_ = 0.0f;
    float jumpCooltime = 0.0f;
    float canJumpCooltime = 0.0f;

  private:
    // 入力操作
    void HandleInput(bool title = false);
    // 全ての衝突判定とプレイヤーの押し戻し
    void CheckCollisionAndResolve(bool title = false);

    void AnimaUpdate(bool title);

    void InversMove(); // 反転時の回転処理

    void CoinParticle(); // コイン取得時のパーティクル

    void RunParitcle(); // 歩いてるときのパーティクル

    // 衝突判定
    /*void OnCollision([[maybe_unused]] Collider* other)override;*/

    // マップチップフィールドを保持
    MapChipField *mapChipField_ = nullptr;
    // マップとの当たり判定情報を返す
    CollisionMapInfo GetMapCollisionInfo(bool title);

    // 反転可能範囲のAABBを描画
    void DrawInvertArea();
    // 反転可能範囲画像をプレイヤーの座標にセット
    void InvertAreaSpriteToPlayerPosition(const ViewProjection &viewProjection);
    // 反転可能範囲画像サイズを現在の範囲によって変更（ごり押しで）
    void InvertAreaSpriteAdjust();

    // ゴールガイド画像をプレイヤーの座標にセット
    void GoalGuideSpriteToPlayerPosition(const ViewProjection &viewProjection);
    void UpdateGoalGuideSpriteAlpha();
    float goalGuideAlpha_ = 0.0f;
    const float alphaIncreaseSpeed = 0.05f; // 徐々に透明度を上げるスピード
    const float alphaDecreaseSpeed = 0.05f; // 徐々に透明度を下げるスピード

    // プレイヤーがコインオブジェクトに触れたかを判定
    bool IsCollidingCoin(const Coin &coin);
    // 取得したコインの座標を保存しておく
    Vector3 lastCollectedCoinPosition_ = {0.0f, 0.0f, 0.0f};

    // 反転操作が無効の際、反転枠を揺らす用
    const float kShakeDuration = 0.5f; // シェイクの継続時間（秒）
    float spriteShakeTimer_;           // シェイクの管理時間
    float spriteShakeOffset_;          // シェイクによる位置のずれ

    // 反転成立時、反転枠の拡縮アニメーションを行う用
    const float kSpriteScaleDuration = 0.4f; // 拡縮時間（秒）
    float spriteScaleTimer_;                 // 拡縮の管理時間

    void SpritePlayerAreaAnimation();

  private:
    using json = nlohmann::json;

    void SaveToJson();
    void LoadFromJson();

    ///
    /// SE・エフェクト用のフラグ
    ///
  private:
    // ジャンプした瞬間を判定
    bool IsJumpOccurred() { return isJumpOccurred_; }
    // ブロック反転した瞬間を判定
    bool IsBlockInversionOccurred() { return isBlockInversionOccurred_; }
    // リセットした瞬間を判定
    bool IsResetOccurred() { return isResetOccurred_; }
    // 重力反転した瞬間を判定
    bool IsGravityReversedOccurred() { return isGravityReversedOccurred_; }
    // 着地した瞬間を判定
    bool IsLandedOccurred();
    // 歩いているかどうかの判定
    bool IsWalking();
    // コインを取得した瞬間を判定
    bool IsCollectCoinOccurred() { return isCollectCoinOccurred_; }
    // 最後に取得したコインの座標を返す
    Vector3 GetLastCollectedCoinPosition() { return lastCollectedCoinPosition_; }
    // 反転操作が無効の瞬間を判定
    bool IsInvertDisabled() { return isInvertDisabled_; }
    // ゴールに触れていて、地面にいる間ずっと（ゴールボタンの表示とかに使用）
    bool IsTouchGoalAndOnGround() { return isTouchGoal_ && collisionMapInfo_.hittingGround_; }

  private:
    // ジャンプした瞬間を判定
    bool isJumpOccurred_ = false;
    bool isJump_ = false;
    // ブロック反転した瞬間を判定
    bool isBlockInversionOccurred_ = false;
    // リセットした瞬間を判定
    bool isResetOccurred_ = false;
    // 重力反転した瞬間を判定
    bool isGravityReversedOccurred_ = false;

    // 前フレームの接地状態を記録
    bool prevHittingGround_ = false;

    bool isWalking_ = false;

    bool isLanded_ = false;

    // コインを取得した瞬間を判定
    bool isCollectCoinOccurred_ = false;
    // 反転操作が無効の場合を判定
    bool isInvertDisabled_ = false;
    // ゴールに触れている間を判定
    bool isTouchGoal_ = false;
    // 反転ボタン押したかどうか
    bool isInvert_ = false;
    // 埋まっている状態でisInvertがtrueになったことを記録
    bool wasInvertWhileOverlapping_ = false;

    // スプライト表示フラグ
    bool shouldDisplay_ = false;

    // スプライト表示タイマー
    float displayTimer_ = 0.0f;

// 埋まってます画像表示用
private:
    // "ブロックにうまっています" スプライト
    std::unique_ptr<Sprite> spritePlayerFilled_;

    float fillAlpha_ = 0.0f; // 透明度（0.0f ~ 1.0f）;
    float fllAlphaSpeed_ = 2.0f; // a変化速度（秒）
 
    float overlapTimer_ = 0.0f; // 埋まっている時間
    bool wasOverlapping_ = false; // 前フレームの状態記録
};