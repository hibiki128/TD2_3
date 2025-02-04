#pragma once
#include "Input.h"
#include "Sprite.h"
#include "application/Base/BaseObject.h"
class Player;
class Pause {
  public:
    /// ===================================================
    /// public method
    /// ===================================================

    void Init();
    void Update();
    void Draw(const ViewProjection &vp);

    bool IsPause() { return isPause_; };
    int GetItem() { return currentItem_; }

    void SetPlayer(Player *player) { player_ = player; }
    void SetStageNum(int num) { stageNum_ = num; }
    bool IsFinish() { return isFinish_; }

  private:
    /// ===================================================
    /// private method
    /// ===================================================

    void InitText();

    void UpdateText();

    void MoveText();

    void OpenMenu();

    void MenuOperation();

    void Debug();

    void InitNumbers();

  private:
    /// ===================================================
    /// private variaus
    /// ===================================================

    Input *input_ = nullptr;

    std::unique_ptr<Sprite> backGround_; //  ポーズ中の背景
    std::unique_ptr<Sprite> backGame_;   // 「ゲームへ戻る」の文字
    std::unique_ptr<Sprite> backSelect_; // 「セレクトへ戻る」の文字
    std::unique_ptr<Sprite> Restart_;    // 「リスタート」の文字
    std::unique_ptr<Sprite> Stage_;      // 「ステージ」の文字
    std::unique_ptr<Sprite> Pointer_;    // 「ポインター」
    std::unique_ptr<Sprite> singleDigit_;
    std::unique_ptr<Sprite> twoDigit_;

    Player *player_ = nullptr;

    Vector4 color_ = {1.0f, 1.0f, 1.0f, 0.0f}; //  ポーズ中の背景の色

    Vector2 backGamePos_ = {0.0f, 0.0f};    // 「ゲームへ戻る」の位置
    Vector2 backSelectPos_ = {0.0f, 0.0f};  // 「セレクトへ戻る」の位置
    Vector2 restartPos_ = {0.0f, 0.0f};     // 「リスタート」の位置
    Vector2 stagePos_ = {0.0f, 0.0f};       // 「ステージ」の位置
    Vector2 pointerPos_ = {0.0f, 0.0f};     // 「ポインター」の位置
    Vector2 spritePosition_ = {0.0f, 0.0f}; //  ポーズ中の背景の位置
    Vector2 backGameSize_;                  // 「ゲームへ戻る」のサイズ
    Vector2 backSelectSize_;                // 「セレクトへ戻る」のサイズ
    Vector2 restartSize_;                   // 「リスタート」のサイズ
    Vector2 stageSize_;                     // 「ステージ」のサイズ
    Vector2 pointerSize_;
    Vector2 singlePos_ = {0.0f, 0.0f};
    Vector2 twoPos_ = {0.0f, 0.0f};
    Vector2 singleSize_;
    Vector2 twoSize_;

    int currentItem_ = 0; //  現在選択しているメニュー項目
    int stageNum_ = 0;

    bool isPause_ = false;         //  ポーズしてるかどうか
    bool CanEscape_ = false;       //  エスケープのクールタイム用
    bool previousIsPause_ = false; //  クラスメンバに前フレームのポーズ状態を保持する変数を追加
    bool textMovedRight_ = false;  //  全部のテキストが右に行ったかどうか
    bool prevEscapeState_ = false;
    bool prevStartState_ = false;
    bool isFinish_ = true;

    struct EasingValue {
        Vector2 start_;
        Vector2 end_;
        float T_;
        float TMax_;
    };

    EasingValue alpha_E;      //  アルファのイージング変数
    EasingValue backGame_E;   // 「ゲームへ戻る」のイージング変数
    EasingValue backSelect_E; // 「セレクトへ戻る」のイージング変数
    EasingValue restart_E;    // 「リスタート」のイージング変数
    EasingValue stage_E;      // 「ステージ」のイージング変数
    EasingValue pointer_E;    // 「ポインター」のイージング変数
    EasingValue single_E;     // 「ポインター」のイージング変数
    EasingValue two_E;        // 「ポインター」のイージング変数

    float EscapeCoolTime_ = 0.0f; //  Escキーのクールタイム
    float pointerYT_;
    float transitionTimer_ = 0.0f; // 経過時間を保持する変数
    float deltaTime_ = 1.0f / 60.0f;
};
