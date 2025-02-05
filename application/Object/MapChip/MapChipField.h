#pragma once

// C++
#include <memory>
#include <stdint.h>
#include <vector>

// Application
#include "application/Base/BaseObject.h"
#include "application/Object/MapChip/Block/Block.h"
// #include "application/Object/Goal/Goal.h"
#include "application/Object/Coin/Coin.h"
#include <ParticleEmitter.h>

class MapChipField {
  private:
    // マップチップのデータ構造
    struct MapChip {
        std::unique_ptr<Block> object;
        std::unique_ptr<ParticleEmitter> normal_;
        std::unique_ptr<ParticleEmitter> arrow_;

        ///
        /// アニメーション関連
        ///
        float animationTime = 0.0f;
        float currentRotation = 0.0f;
        float delayTime = 0.0f;

        bool isAnimating = false;
        bool isDelaying = false;
        bool hasColorChanged = false;

        // このアニメーションが「プレイヤー色変更ブロック由来」か否か
        bool isColorChangeAnimation = false;

        enum class AnimationState { None,
                                    Shrinking,
                                    ColorChange,
                                    Expanding } animState = AnimationState::None;

        float currentScale = 1.0f;
    };

  public:
    size_t mapWidth = 13;         // 横マス数
    size_t mapHeight = 8;         // 縦マス数
    static const float kChipSize; // 各マップチップのサイズ

    ///
    /// 基本的な関数
    ///

    MapChipField();

    void Init(const std::string &csvFilePath);
    void Update(const Vector3 &center, int xRange, int yRange);
    void Draw(const ViewProjection &vp);
    void DebugImGui();
    void DrawParticle(const ViewProjection &vp);
    void PlaySE();
    void SetGoalModel() {
        for (const auto &row : mapChips_) {
            for (const auto &chip : row) {
                if (chip.object->type_ == Block::ChipType::Goal) {
                    chip.object->SetModel("game/door.obj");
                }
            }
        }
    }

    // 全てのブロックのBaseObjectポインタを取得
    std::vector<Block *> GetBlocks() const;
    // ゴールオブジェクトの取得
    Vector3 GetGoalPosition() const { return goalPosition_; }
    // コインオブジェクトの取得
    std::vector<std::unique_ptr<Coin>> &GetCoins() { return coins_; }
    // プレイヤー初期位置を返す
    Vector3 GetPlayerInitialPosition() { return playerInitialPosition_; }
    // マップの状態を初期状態に戻す
    void ResetMapChip();

    ///
    ///	ブロックの反転処理（プレイヤー側で呼び出す）
    ///

    // プレイヤー範囲内のブロックの色を反転（
    void InvertBlocksInArea(const Vector3 &center, int xRange, int yRange);
    // 反転させて挟み込んだブロックの色の反転を行う
    void InvertBlocksWithCapture();

    ///
    ///	その他
    ///

    // プレイヤー範囲内のブロックを拡大->縮小させる
    void UpdateBlockScaleAnimation(const Vector3 &center, int xRange, int yRange);

    // 指定範囲内にブロックが存在しているかを判定
    bool HasBlockInArea(const Vector3 &center, int xRange, int yRange);

    // 指定範囲内に重力反転ブロックがあるかどうかを判定
    bool HasGravityBlockInArea(const Vector3 &center, int xRange, int yRange);
    // マップチップフィールドが所持する重力反転状態を設定（重力ブロックのテクスチャ変更のためだけに使用）
    void SetIsGravityReversed(bool flag) { isGravityReversed_ = flag; }

    // 指定範囲内にプレイヤー色反転ブロックがあるかどうかを判定
    bool HasColorChangeBlockInArea(const Vector3 &center, int xRange, int yRange);

    // プレイヤーの色状態を確認してセットする
    void SetIsPlayerWhite(bool flag) { isPlayerWhite_ = flag; }

    // 1つでもアニメーション中のブロックがあるかどうかを探索
    bool IsAnyChipAnimating() const;

  private:
    // マップチップの二次元配列
    std::vector<std::vector<MapChip>> mapChips_;
    std::string csvFilePath_; // ファイルパス保存用

    // ゴールオブジェクト
    /*std::unique_ptr<Goal> goal_;*/
    // ゴール位置を格納
    Vector3 goalPosition_;
    // プレイヤー初期位置を格納
    Vector3 playerInitialPosition_;

    // コインオブジェクト
    std::vector<std::unique_ptr<Coin>> coins_;

  private:
    ///
    ///	CSVファイルからマップチップの読み込み
    ///

    // CSVからマップチップデータを読み込む
    void LoadFromCSV(const std::string &filePath);
    // ChipTypeを整数値から取得する関数
    Block::ChipType GetChipTypeFromInt(int value);

    ///
    ///	ブロックの挟み込み反転処理
    ///

    // 挟み込み処理を汎用化
    void ProcessCapture(int startX, int startY, Block::ChipType targetType, Block::ChipType ownType, const std::vector<std::pair<int, int>> &directions);
    // 指定された座標のブロックを反転する
    void InvertBlock(int x, int y);
    // 指定された座標が有効範囲内か確認
    bool IsValidPosition(int x, int y) const;

    // 初期状態で挟み込みが起きないよう、プレイヤーが一度でも反転を行ったかを記録
    /*bool hasPlayerInverted_ = false;*/

    // 重力反転時のパーティクル用
    void GravityParticleUpdate();

    ///
    ///	アニメーション関連
    ///

    // ブロックの色反転時に { 縮小->色反転->拡大 } を行うアニメーション
    void UpdateChipAnimation(MapChip &chip);

    ///
    /// その他
    ///

    // 存在する全ての重力ブロックのテクスチャを重力状態によって変更
    void ChangeTextureAllGravityBlock();

  private:
    // 重力反転状態かどうか（重力ブロックのテクスチャ変更のためだけに使用）
    bool isGravityReversed_ = false; // 初期状態は通常

    // プレイヤーが白いかどうか（通常ブロックとスカスカブロックの変更のためだけに使用）
    bool isPlayerWhite_ = true;
    bool prevGravityState = false;
    float arrowTime_ = 0.0f;
};