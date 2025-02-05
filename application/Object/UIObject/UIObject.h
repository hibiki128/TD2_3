#pragma once

#include "Sprite.h"
#include "application/Base/BaseObject.h"

// Engine
#include "Input.h"

class UIObject {
  public:
    void Init();
    void Update();
    void Draw(const ViewProjection &viewProjection);
    void DrawSprite();
    void DebugImGui();

  private:
    // 入力
    Input *input_;

    ///
    /// オブジェクト
    ///
    std::unique_ptr<BaseObject> objectBook_; // 本
    std::unique_ptr<BaseObject> objectUI_;   // 栞
    //std::unique_ptr<BaseObject> filter_;

    std::unique_ptr<BaseObject> objectL_; // Lスティック
    std::unique_ptr<BaseObject> objectA_; // Aボタン
    std::unique_ptr<BaseObject> objectR_; // RBボタン

    ///
    ///	スプライト
    ///
    std::unique_ptr<Sprite> spritePause_; // ポーズボタン

  private:
    // Lスティックオブジェクトの初期位置
    Vector3 initLstickPos_;

    // パッド入力による反応
    void InputReaction();

    Vector4 filterColor_;
};
