#include "MapChipField.h"

// C++
#include <fstream>
#include <iostream>
#include <sstream>

// Engine
#include "ParticleEditor.h"
#include "math/Easing.h"
#include "myEngine/utility/graphics/TextureManager.h"
#include <ParticleCommon.h>
#include <loadFile/csv/CsvLoad.h>
#include <myEngine/Frame/Frame.h>

// ブロックの大きさを定義
const float MapChipField::kChipSize = 2.0f;

MapChipField::MapChipField() {}

void MapChipField::Init(const std::string &csvFilePath) {
    // 初期状態で挟み込みが起きないようにするフラグ
    /*hasPlayerInverted_ = false;*/

    // ファイルパスの保存をしておく
    csvFilePath_ = csvFilePath;

    // CSVファイルからマップの読み込み
    LoadFromCSV(csvFilePath_);

    InitParticle();
}

void MapChipField::InitParticle() {
    //// 既存のパーティクルをクリア（リセット処理）
    // normal_.clear();
    // arrow_.clear();
    // goal_.clear();

    // for (auto &row : mapChips_) {
    //     for (auto &chip : row) {
    //         // 空白ブロックでない場合のみ処理
    //         if (chip.object->type_ != Block::ChipType::Empty) {
    //             std::unique_ptr<ParticleEmitter> particle = std::make_unique<ParticleEmitter>();

    //            // 各タイプごとに異なる初期化を行い、対応するリストに追加
    //            if (chip.object->type_ == Block::ChipType::Black || chip.object->type_ == Block::ChipType::White) {
    //                particle->Initialize("reverse", "debug/sphere.obj");
    //                normal_.push_back(std::move(particle));
    //            } else if (chip.object->type_ == Block::ChipType::Gravity) {
    //                particle->Initialize("arrow_up", "debug/plane.obj");
    //                particle->SetTexture("Particle/Arrow.png");
    //                arrow_.push_back(std::move(particle));
    //            } else if (chip.object->type_ == Block::ChipType::Goal) {
    //                particle->Initialize("goal", "debug/plane.obj");
    //                particle->SetTexture("particle/circle.png");
    //                goal_.push_back(std::move(particle));
    //            }
    //        }
    //    }
    //}
}

void MapChipField::Update(const Vector3 &center, int xRange, int yRange, bool title) {
    for (auto &row : mapChips_) {
        for (auto &chip : row) {
            // 空白ブロックではない場合のみ更新
            if (chip.object->type_ != Block::ChipType::Empty) {
                // アニメーションの進行
                if (chip.isAnimating) {
                    UpdateChipAnimation(chip);
                    // 通常の更新
                } else {
                    chip.object->Update();
                }
            }
        }
    }

    // プレイヤー範囲内のブロックを半透明にする
    UpdateBlockScaleAnimation(center, xRange, yRange);

    // 挟み込みが起こった場合に挟まれたブロックの色反転を行う
    InvertBlocksWithCapture();

    GravityParticleUpdate();
    GoalParticleUpdate(title);

    /*if (hasPlayerInverted_) {
            InvertBlocksWithCapture();
    }*/

    // ゴールオブジェクト更新
    /*goal_->Update();*/
    // コインオブジェクト更新
    for (auto &coin : coins_) {
        coin->Update();

        // 取得済みのコインを削除
        if (coin->IsCollected()) {
            coin.reset();
        }
    }
    // 取得済みのコインを削除する（resetでnullptrになったコインの削除）
    coins_.erase(std::remove_if(coins_.begin(), coins_.end(), [](const std::unique_ptr<Coin> &coin) { return !coin; }), coins_.end());
}

void MapChipField::Draw(const ViewProjection &vp) {
    for (const auto &row : mapChips_) {
        for (const auto &chip : row) {
            // 空白ブロックではない場合のみ描画
            if (chip.object->type_ != Block::ChipType::Empty) {
                chip.object->Draw(vp);
            }
        }
    }

    // ゴールオブジェクト描画
    /*goal_->Draw(vp);*/
    // コインオブジェクト描画
    for (auto &coin : coins_) {
        coin->Draw(vp);
    }
}

void MapChipField::DebugImGui() {
    ImGui::Begin("MapChipField");

    ImGui::Checkbox("重力反転状態", &isGravityReversed_);
    ImGui::Checkbox("プレイヤーは白い？", &isPlayerWhite_);

    const auto blocks = GetBlocks();
    for (const auto &block : blocks) {
        ImGui::Text("Position : (%.1f, %.1f, %.1f) type : %d", block->GetWorldPosition().x, block->GetWorldPosition().y, block->GetWorldPosition().z, block->type_);
    }

    ImGui::End();
}

void MapChipField::DrawParticle(const ViewProjection &vp) {

    for (const auto &row : mapChips_) {
        for (const auto &chip : row) {
            // 空白ブロックではない場合のみ描画
            if (chip.object->type_ == Block::ChipType::Black || chip.object->type_ == Block::ChipType::White) {
                ParticleCommon::GetInstance()->SetBlendMode(BlendMode::kNormal);
                chip.normal_->Draw(vp);
            }
            if (chip.object->type_ == Block::ChipType::Gravity) {
                ParticleCommon::GetInstance()->SetBlendMode(BlendMode::kAdd);
                chip.arrow_->Draw(vp);
            }
            if (chip.object->type_ == Block::ChipType::Goal) {
                ParticleCommon::GetInstance()->SetBlendMode(BlendMode::kAdd);
                chip.goal_->Draw(vp);
            }
        }
    }
}

void MapChipField::PlaySE() {
    // for (auto& row : mapChips_) {
    //	for (auto& chip : row) {
    //		if (chip.isInvers) {
    //			Audio::GetInstance()->PlayWave(invertSE_, 0.1f);
    //			chip.isInvers = false;
    //		}
    //	}
    //}
}

std::vector<Block *> MapChipField::GetBlocks() const {
    std::vector<Block *> blocks;

    for (const auto &row : mapChips_) {
        for (const auto &chip : row) {
            if (chip.object) {
                blocks.push_back(chip.object.get());
            }
        }
    }

    return blocks;
}

void MapChipField::ResetMapChip() {
    // マップチップの二次元配列をクリアする
    mapChips_.clear();

    // コインの配列もクリアする
    coins_.clear();

    // CSVファイルからマップの読み込み
    LoadFromCSV(csvFilePath_);

    // 全てのブロックの状態をリセット
    for (auto &row : mapChips_) {
        for (auto &chip : row) {
            chip.isAnimating = false;
            chip.isDelaying = false;
            chip.animState = MapChip::AnimationState::None;
            chip.animationTime = 0.0f;
            chip.delayTime = 0.0f;
            chip.currentScale = 1.0f;
        }
    }
}

void MapChipField::InvertBlocksInArea(const Vector3 &center, int xRange, int yRange) {
    // プレイヤーが反転を行ったことを記録する
    /*hasPlayerInverted_ = true;*/

    // 中心位置からマップ上のマス位置を計算
    int centerX = static_cast<int>(std::round(center.x / kChipSize));
    int centerY = static_cast<int>(std::round(-center.y / kChipSize));

    int halfXRange = xRange / 2;
    int halfYRange = yRange / 2;

    // 3x3マス内のブロックを探索
    for (int y = -halfYRange; y <= halfYRange; ++y) {
        for (int x = -halfXRange; x <= halfXRange; ++x) {
            // 対象位置を計算
            int targetX = centerX + x;
            int targetY = centerY + y;

            // マップ範囲外を無視
            if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
                continue;
            }

            // ブロックを取得して反転処理（アニメーション開始）
            MapChip &chip = mapChips_[targetY][targetX];
            if (!chip.isAnimating && chip.object->type_ != Block::ChipType::Empty && chip.object->type_ != Block::ChipType::Goal) {
                chip.isAnimating = true;
                chip.animState = MapChip::AnimationState::Shrinking;
                chip.animationTime = 0.0f;
            }
        }
    }
}

void MapChipField::LoadFromCSV(const std::string &filePath) {
    // CSVファイルからデータを読み込む（キャッシュを使って効率化）
    auto mapChipData = CsvLoad::GetInstance()->LoadCsv(filePath);

    // マップチップの二次元配列をサイズ調整
    mapChips_.resize(mapChipData.size());

    // 各エミッターのカウンター
    int reverseCount = 1;
    int arrowUpCount = 1;

    for (int y = 0; y < mapChipData.size(); ++y) {
        for (int x = 0; x < mapChipData[y].size(); ++x) {
            int chipValue = mapChipData[y][x];                        // CSVから取得したマップチップ番号
            Block::ChipType chipType = GetChipTypeFromInt(chipValue); // チップ番号からタイプを取得

            // MapChip オブジェクトを作成
            MapChip chip;
            chip.object = std::make_unique<Block>();
            chip.object->type_ = chipType;

            // チップタイプに応じた初期化
            if (chip.object->type_ != Block::ChipType::Empty) {
                chip.object->Init("Block");
                chip.object->SetScale({1.0f, 1.0f, 1.0f});
                chip.object->SetWorldPosition({x * kChipSize, -y * kChipSize, 0.0f});
                chip.object->CreateCollider();

                // 各チップに個別のエミッターを割り当てる
                if (chip.object->type_ == Block::ChipType::White || chip.object->type_ == Block::ChipType::Black) {
                    std::string emitterName = "reverse" + std::to_string(reverseCount);
                    chip.normal_ = ParticleEditor::GetInstance()->GetEmitter(emitterName);

                    std::string emitterName2 = "clip" + std::to_string(reverseCount);
                    chip.clip_ = ParticleEditor::GetInstance()->GetEmitter(emitterName2);

                    reverseCount = (reverseCount % 90) + 1; // 1～40 でループ
                } else if (chip.object->type_ == Block::ChipType::Gravity) {
                    std::string emitterName = "arrow_up" + std::to_string(arrowUpCount);
                    chip.arrow_ = ParticleEditor::GetInstance()->GetEmitter(emitterName);
                    arrowUpCount = (arrowUpCount % 4) + 1; // 1～4 でループ
                } else if (chip.object->type_ == Block::ChipType::Goal) {
                    chip.goal_ = ParticleEditor::GetInstance()->GetEmitter("goal");
                }

                // チップの種類に応じて処理を行う
                switch (chip.object->type_) {
                case Block::ChipType::Black:
                    chip.object->CreateModel("game/blackBlock.obj");
                    chip.object->SetTexture("game/blackBlock.png");
                    break;
                case Block::ChipType::White:
                    chip.object->CreateModel("game/noTouchWhiteBlock.obj");
                    chip.object->SetTexture("game/noTouchWhiteBlock.png");
                    break;
                case Block::ChipType::Gray:
                    chip.object->CreateModel("game/block.obj");
                    chip.object->SetTexture("game/block.png");
                    break;
                case Block::ChipType::Goal:
                    chip.object->CreateModel("game/goal.obj");
                    chip.object->SetTexture("game/goal.png");
                    goalPosition_ = {x * kChipSize, -y * kChipSize, 0.0f};
                    break;
                case Block::ChipType::Gravity:
                    chip.object->CreateModel("game/gravityBlockDown.obj");
                    chip.object->SetTexture("game/gravityBlockDown.png");
                    break;
                case Block::ChipType::ColorChange:
                    chip.object->CreateModel("game/playerSwitchBlockWhite.obj");
                    chip.object->SetTexture("game/playerSwitchBlockWhite.png");
                    break;
                default:
                    break;
                }
            }

            // プレイヤー初期位置の格納
            if (chipValue == 5) {
                playerInitialPosition_ = {x * kChipSize, -y * kChipSize, 0.0f};
            }

            // コインの生成
            if (chipValue == 8) {
                auto coin = std::make_unique<Coin>();
                coin->Init("Coin");
                coin->CreateModel("game/coin.obj");
                coin->SetTexture("game/coin.png");
                coin->SetWorldPosition({x * kChipSize, -y * kChipSize, 0.0f});
                coin->CreateCollider();
                coins_.push_back(std::move(coin));
            }

            // マップチップの二次元配列に格納
            mapChips_[y].push_back(std::move(chip));
        }
    }

    // マップの幅と高さを再設定
    mapWidth = mapChips_[0].size();
    mapHeight = mapChips_.size();
}

Block::ChipType MapChipField::GetChipTypeFromInt(int value) {
    switch (value) {
    case 0:
        return Block::ChipType::Empty;
    case 1:
        return Block::ChipType::Black;
    case 2:
        return Block::ChipType::White;
    case 3:
        return Block::ChipType::Gray;
    case 4:
        return Block::ChipType::Goal;
    case 5:
        return Block::ChipType::Empty; // プレイヤー初期位置は空白扱いとする
    case 6:
        return Block::ChipType::Gravity;
    case 7:
        return Block::ChipType::ColorChange;
    case 8:
        return Block::ChipType::Empty; // コインオブジェクトは空白扱いとする

    default:
        return Block::ChipType::Empty;
    }
}

void MapChipField::InvertBlocksWithCapture() {
    // 8方向を表すオフセット（x, y）
    const std::vector<std::pair<int, int>> directions = {
        {0, -1},
        {0, 1},
        {-1, 0},
        {1, 0}, // 上下左右
                //{-1, -1}, {-1, 1}, {1, -1}, {1, 1} // 斜め方向
    };

    // ステージ全体のブロックを探索
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            // 起点が黒ブロックの場合に白を挟み込む
            if (mapChips_[y][x].object->type_ == Block::ChipType::Black) {
                ProcessCapture(x, y, Block::ChipType::White, Block::ChipType::Black, directions);
            }
            // 起点が白ブロックの場合に黒を挟み込む
            else if (mapChips_[y][x].object->type_ == Block::ChipType::White) {
                ProcessCapture(x, y, Block::ChipType::Black, Block::ChipType::White, directions);
            }
        }
    }
}

void MapChipField::ProcessCapture(int startX, int startY, Block::ChipType targetType, Block::ChipType ownType, const std::vector<std::pair<int, int>> &directions) {
    for (const auto &dir : directions) {
        std::vector<std::pair<int, int>> capturedBlocks;

        // 現在位置から方向に沿って探索
        int currX = startX + dir.first;
        int currY = startY + dir.second;

        while (IsValidPosition(currX, currY)) {
            // 対象タイプのブロックが見つかれば一時的にキャプチャ
            if (mapChips_[currY][currX].object->type_ == targetType) {
                capturedBlocks.emplace_back(currX, currY);
            }
            // 自分のタイプのブロックが見つかれば挟み込み成立
            else if (mapChips_[currY][currX].object->type_ == ownType) {
                for (const auto &block : capturedBlocks) {
                    InvertBlock(block.first, block.second);
                }
                break;
            }
            // 空白や他のタイプのブロックの場合、挟み込み失敗
            else {
                break;
            }

            // 次の位置に進む
            currX += dir.first;
            currY += dir.second;
        }
    }
}

void MapChipField::InvertBlock(int x, int y) {
    // 有効範囲内のブロックのみ
    if (IsValidPosition(x, y)) {
        MapChip &chip = mapChips_[y][x];

        // 動かないブロックの場合はスキップ
        if (chip.object->type_ == Block::ChipType::Gray) {
            return;
        }

        if (!chip.isAnimating && chip.object->type_ != Block::ChipType::Empty) {
            // アニメーション開始の遅延処理
            chip.isDelaying = true;
            chip.isCliping = true;
            chip.delayTime = 0.3f; // ここで指定した時間遅延

            chip.isAnimating = true; // 遅延後にアニメーションを開始するため有効化
        }
    }
}

bool MapChipField::IsValidPosition(int x, int y) const { return x >= 0 && x < static_cast<int>(mapWidth) && y >= 0 && y < static_cast<int>(mapHeight); }

void MapChipField::GravityParticleUpdate() {
    if (isGravityReversed_ != prevGravityState) {
        arrowTime_ = 1.0f;
    }

    if (arrowTime_ > 0.0f) {
        arrowTime_ -= Frame::DeltaTime();
        for (const auto &row : mapChips_) {
            for (const auto &chip : row) {
                if (chip.object->type_ == Block::ChipType::Gravity) {
                    if (isGravityReversed_ && !prevGravityState) {
                        chip.arrow_->SetColor({1.0f, 0.0f, 0.0f, 1.0f});
                        chip.arrow_->LoadFromJson("arrow_up");
                    }
                    if (!isGravityReversed_ && prevGravityState) {
                        chip.arrow_->SetColor({0.0f, 0.0f, 1.0f, 1.0f});
                        chip.arrow_->LoadFromJson("arrow_down");
                    }
                    chip.arrow_->SetPosition({chip.object->GetCenterPosition().x, chip.object->GetCenterPosition().y, -1.5f});
                    // 重力ブロックのみ
                    chip.arrow_->Update();
                }
            }
        }
    }

    // 現在の状態を前回の状態として保存
    prevGravityState = isGravityReversed_;
}

void MapChipField::GoalParticleUpdate(bool title) {

    if (!title) {
        for (const auto &row : mapChips_) {
            for (const auto &chip : row) {
                if (chip.object->type_ == Block::ChipType::Goal) {
                    chip.goal_->SetPosition({chip.object->GetCenterPosition().x - 0.5f, chip.object->GetCenterPosition().y, -1.5f});
                    chip.goal_->SetColor({1.0f, 1.0f, 0.0f, 1.0f});
                    chip.goal_->Update();
                }
            }
        }
    }
}

void MapChipField::UpdateChipAnimation(MapChip &chip) {
    // 動かないブロックの場合はスキップ
    if (chip.object->type_ == Block::ChipType::Gray) {
        chip.isAnimating = false; // 念のためアニメーション状態のリセット
        chip.animState = MapChip::AnimationState::None;
        return;
    }

    ///
    /// アニメーション開始の遅延処理
    ///

    if (chip.isDelaying) {
        chip.delayTime -= 1.0f / 60.0f; // フレーム減少

        if (chip.delayTime <= 0.0f) {
            chip.isDelaying = false;                             // 遅延終了
            chip.animState = MapChip::AnimationState::Shrinking; // 回転アニメーションに移行
            chip.animationTime = 0.0f;
        }
        return; // 遅延中はここで処理を終了する
    }

    constexpr float rotationDuration = 0.4f;                    // 回転アニメーションの時間
    constexpr float halfRotationTime = rotationDuration / 2.0f; // 半回転のタイミング

    ///
    /// アニメーション処理開始
    ///

    chip.animationTime += 1.0f / 60.0f; // フレーム進行

    /* ブロックの一回転 -> 拡縮 -> 半回転時に色変更 -> 終了 の順番で処理を行う */

    ///
    /// ブロックの回転およびスケールアニメーション
    ///
    if (chip.animState == MapChip::AnimationState::Shrinking) {
        // 終了した場合
        if (chip.animationTime >= rotationDuration) {
            chip.animationTime = 0.0f;                      // タイマーリセット
            chip.animState = MapChip::AnimationState::None; // 次の状態に移行
            chip.isAnimating = false;                       // アニメーション終了
            chip.hasColorChanged = false;
            chip.isColorChangeAnimation = false;

            // 最終的に回転角度とスケールをリセットして終了
            chip.currentRotation = 0.0f;
            chip.currentScale = 1.0f;
            chip.object->SetRotation({0.0f, chip.currentRotation, 0.0f});
            chip.object->SetScale({chip.currentScale, chip.currentScale, chip.currentScale});

            if (chip.object->type_ == Block::ChipType::Black) {
                chip.normal_->SetPosition(chip.object->GetCenterPosition());
                chip.normal_->SetTexture("particle/blackBlock1x1.png");
            }
            if (chip.object->type_ == Block::ChipType::White) {
                chip.normal_->SetPosition(chip.object->GetCenterPosition());
                chip.normal_->SetTexture("particle/whiteBlock1x1.png");
            }

            if (chip.object->type_ == Block::ChipType::Black || chip.object->type_ == Block::ChipType::White) {
                chip.normal_->UpdateOnce();
            }
            if (chip.isCliping) {
                // 挟まれたとき用演出

                chip.isCliping = false;
            }
        } else {
            // 回転角度を更新
            chip.currentRotation = EaseOutQuad(0.0f, 720.0f, chip.animationTime, rotationDuration); // 回転を0° -> 360°へ
            chip.object->SetRotation({0.0f, degreesToRadians(chip.currentRotation), 0.0f});

            // スケールの更新
            if (chip.animationTime <= halfRotationTime) {
                // 半回転までは縮小
                chip.currentScale = EaseOutQuad(1.0f, 0.5f, chip.animationTime, halfRotationTime); // スケールを1.0 -> 0.5へ縮小
            } else {
                // 半回転以降は拡大
                float timeSinceHalf = chip.animationTime - halfRotationTime;
                chip.currentScale = EaseOutQuad(0.5f, 1.0f, timeSinceHalf, halfRotationTime); // スケールを0.5 -> 1.0へ拡大
            }
            chip.object->SetScale({chip.currentScale, chip.currentScale, chip.currentScale});
            // 半回転のタイミングで色を変更
            if (chip.animationTime >= halfRotationTime && !chip.hasColorChanged) {
                chip.hasColorChanged = true; // 色変更が一度だけ行われるようにフラグを設定

                ///
                /// プレイヤー色反転ブロック由来の場合、ブロックのタイプは変更しないで、通常->スカスカのみを変更する
                ///
                if (chip.isColorChangeAnimation) {
                    // プレイヤーが白い場合には白ブロックをスカスカに、黒ブロックを通常に
                    if (isPlayerWhite_) {
                        // ブロックの色変更
                        if (chip.object->type_ == Block::ChipType::White) {
                            chip.object->CreateModel("game/noTouchWhiteBlock.obj");
                            chip.object->SetTexture("game/noTouchWhiteBlock.png");
                        } else if (chip.object->type_ == Block::ChipType::Black) {
                            chip.object->CreateModel("game/blackBlock.obj");
                            chip.object->SetTexture("game/blackBlock.png");
                        }
                        // プレイヤーが黒い場合には黒ブロックをスカスカに、白ブロックを通常に
                    } else {
                        // ブロックの色変更
                        if (chip.object->type_ == Block::ChipType::Black) {
                            chip.object->CreateModel("game/noTouchBlackBlock.obj");
                            chip.object->SetTexture("game/noTouchBlackBlock.png");
                        } else if (chip.object->type_ == Block::ChipType::White) {
                            chip.object->CreateModel("game/whiteBlock.obj");
                            chip.object->SetTexture("game/whiteBlock.png");
                        }
                    }
                    ///
                    /// 色変更を伴う反転の場合には、ブロックのtypeを変更
                    ///
                } else {
                    // プレイヤーが白い場合には白ブロックをスカスカに、黒ブロックを通常に
                    if (isPlayerWhite_) {
                        // ブロックの色変更
                        if (chip.object->type_ == Block::ChipType::Black) {
                            chip.object->type_ = Block::ChipType::White;
                            chip.object->CreateModel("game/noTouchWhiteBlock.obj");
                            chip.object->SetTexture("game/noTouchWhiteBlock.png"); // 白ブロックのテクスチャをセット
                        } else if (chip.object->type_ == Block::ChipType::White) {
                            chip.object->type_ = Block::ChipType::Black;
                            chip.object->CreateModel("game/blackBlock.obj");
                            chip.object->SetTexture("game/blackBlock.png"); // 黒ブロックのテクスチャをセット
                        }
                        // プレイヤーが黒い場合には黒ブロックをスカスカに、白ブロックを通常に
                    } else {
                        // ブロックの色変更
                        if (chip.object->type_ == Block::ChipType::Black) {
                            chip.object->type_ = Block::ChipType::White;
                            chip.object->CreateModel("game/whiteBlock.obj");
                            chip.object->SetTexture("game/whiteBlock.png"); // 白ブロックのテクスチャをセット
                        } else if (chip.object->type_ == Block::ChipType::White) {
                            chip.object->type_ = Block::ChipType::Black;
                            chip.object->CreateModel("game/noTouchBlackBlock.obj");
                            chip.object->SetTexture("game/noTouchBlackBlock.png"); // 黒ブロックのテクスチャをセット
                        }
                    }
                }

                // プレイヤーが白い場合には
                if (isPlayerWhite_) {
                    // プレイヤー色変更ブロックを白くする
                    if (chip.object->type_ == Block::ChipType::ColorChange) {
                        chip.object->CreateModel("game/playerSwitchBlockWhite.obj");
                        chip.object->SetTexture("game/playerSwitchBlockWhite.png");
                    }
                    // プレイヤーが黒い場合には
                    // プレイヤー色変更ブロックを黒くする
                } else {
                    if (chip.object->type_ == Block::ChipType::ColorChange) {
                        chip.object->CreateModel("game/playerSwitchBlockBlack.obj");
                        chip.object->SetTexture("game/playerSwitchBlockBlack.png");
                    }
                }
            }
        }
    }
}

void MapChipField::ChangeTextureAllGravityBlock() {

    // 全てのブロックを探索
    for (auto &row : mapChips_) {
        for (auto &chip : row) {
            if (chip.object->type_ == Block::ChipType::Gravity) { // 重力ブロックの場合
                if (!isGravityReversed_) {
                    chip.object->CreateModel("game/gravityBlockDown.obj"); // 重力通常状態のテクスチャを設定
                    chip.object->SetTexture("game/gravityBlockDown.png");  // 重力通常状態のテクスチャを設定
                } else {
                    chip.object->CreateModel("game/gravityBlockUp.obj"); // 重力反転状態のテクスチャを設定
                    chip.object->SetTexture("game/gravityBlockUp.png");  // 重力反転状態のテクスチャを設定
                }
            }
        }
    }
}

void MapChipField::UpdateBlockScaleAnimation(const Vector3 &center, int xRange, int yRange) {
    constexpr float kMinScale = 1.0;
    constexpr float kMaxScale = 1.075f;
    constexpr float kCycleDuration = 0.75f; // 拡大と縮小の一周期の時間

    const float kDeltaTime = 1.0f / 60.0f;

    static float globalAnimTime = 0.0f;
    globalAnimTime += kDeltaTime;

    // 周期毎にリセット（0 ~ 1になるように）
    float tCycle = std::fmod(globalAnimTime, kCycleDuration) / kCycleDuration;

    float animatedScale = 0.0f;
    // 拡大中の処理（0.0f ~ 0.5f）
    if (tCycle < 0.5f) {
        float t = tCycle * 2.0f; // 0 ~ 1に正規化
        animatedScale = EaseInQuad<float>(kMinScale, kMaxScale, t, 1.0f);
        // 縮小中の処理（0.5f ~ 1.0f）
    } else {
        float t = (tCycle - 0.5f) * 2.0f; // 0 ~ 1に正規化
        animatedScale = EaseInQuad<float>(kMaxScale, kMinScale, t, 1.0f);
    }

    // 最初に、全てのブロックの透明度を通常に戻す
    for (size_t y = 0; y < mapHeight; ++y) {
        for (size_t x = 0; x < mapWidth; ++x) {
            MapChip &chip = mapChips_[y][x];
            // 対象外のブロックはスキップ
            if (chip.object->type_ == Block::ChipType::Empty ||
                chip.object->type_ == Block::ChipType::Gray ||
                chip.object->type_ == Block::ChipType::Goal) {
                continue;
            }
            if (!chip.isAnimating) {
                chip.object->SetScale({kMinScale, kMinScale, kMinScale});
            }
        }
    }

    // 中心位置からマップ上のマス位置を計算
    int centerX = static_cast<int>(std::round(center.x / kChipSize));
    int centerY = static_cast<int>(std::round(-center.y / kChipSize));

    int halfXRange = xRange / 2;
    int halfYRange = yRange / 2;

    // 範囲内のブロックを探索
    for (int y = -halfYRange; y <= halfYRange; ++y) {
        for (int x = -halfXRange; x <= halfXRange; ++x) {
            // 対象位置を計算
            int targetX = centerX + x;
            int targetY = centerY + y;

            // マップ範囲外を無視
            if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
                continue;
            }

            // 対象外のブロックはスキップ
            MapChip &chip = mapChips_[targetY][targetX];
            if (chip.object->type_ != Block::ChipType::Empty &&
                chip.object->type_ != Block::ChipType::Gray &&
                chip.object->type_ != Block::ChipType::Goal) {
                if (!chip.isAnimating) {
                    chip.object->SetScale({animatedScale, animatedScale, animatedScale});
                }
            }
        }
    }
}

bool MapChipField::HasBlockInArea(const Vector3 &center, int xRange, int yRange) {
    // 中心位置からマップ上のマス位置を計算
    int centerX = static_cast<int>(std::round(center.x / kChipSize));
    int centerY = static_cast<int>(std::round(-center.y / kChipSize));

    int halfXRange = xRange / 2;
    int halfYRange = yRange / 2;

    // 範囲内のブロックを探索
    for (int y = -halfYRange; y <= halfYRange; ++y) {
        for (int x = -halfXRange; x <= halfXRange; ++x) {
            // 対象位置を計算
            int targetX = centerX + x;
            int targetY = centerY + y;

            // マップ範囲外を無視
            if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
                continue;
            }

            // マスにブロックが存在するかをチェック
            MapChip &chip = mapChips_[targetY][targetX];
            if (chip.object->type_ != Block::ChipType::Empty && // 空ブロックを除外する
                chip.object->type_ != Block::ChipType::Gray &&
                chip.object->type_ != Block::ChipType::Goal) { // 動かないブロックを除外する（あとで変更する可能性あり）
                return true;
            }
        }
    }

    return false;
}

bool MapChipField::HasGravityBlockInArea(const Vector3 &center, int xRange, int yRange) {
    // 中心位置からマップ上のマス位置を計算
    int centerX = static_cast<int>(std::round(center.x / kChipSize));
    int centerY = static_cast<int>(std::round(-center.y / kChipSize));

    int halfXRange = xRange / 2;
    int halfYRange = yRange / 2;

    // 範囲内のブロックを探索
    for (int y = -halfYRange; y <= halfYRange; ++y) {
        for (int x = -halfXRange; x <= halfXRange; ++x) {
            // 対象位置を計算
            int targetX = centerX + x;
            int targetY = centerY + y;

            // マップ範囲外を無視
            if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
                continue;
            }

            // ブロックを取得してタイプを判定
            MapChip &chip = mapChips_[targetY][targetX];
            if (chip.object->type_ == Block::ChipType::Gravity) { // 重力ブロックが見つかったら
                isGravityReversed_ = !isGravityReversed_;         // 重力の状態を切り替える

                // 重力の状態によって全ての重力ブロックのテクスチャを切り替える
                ChangeTextureAllGravityBlock();

                return true; // 重力ブロックが見つかったことを示す
            }
        }
    }

    return false;
}

bool MapChipField::HasColorChangeBlockInArea(const Vector3 &center, int xRange, int yRange) {
    // 中心位置からマップ上のマス位置を計算
    int centerX = static_cast<int>(std::round(center.x / kChipSize));
    int centerY = static_cast<int>(std::round(-center.y / kChipSize));

    int halfXRange = xRange / 2;
    int halfYRange = yRange / 2;

    // 範囲内のブロックを探索
    for (int y = -halfYRange; y <= halfYRange; ++y) {
        for (int x = -halfXRange; x <= halfXRange; ++x) {
            // 対象位置を計算
            int targetX = centerX + x;
            int targetY = centerY + y;

            // マップ範囲外を無視
            if (targetX < 0 || targetX >= static_cast<int>(mapWidth) || targetY < 0 || targetY >= static_cast<int>(mapHeight)) {
                continue;
            }

            // ブロックを取得してタイプを判定
            MapChip &chip = mapChips_[targetY][targetX];
            if (chip.object->type_ == Block::ChipType::ColorChange) { // プレイヤー色反転ブロックが見つかったら
                //// 全てのブロックを探索
                // for (auto &row : mapChips_) {
                //     for (auto &chip : row) {
                //         if (!chip.isAnimating) {
                //             // 白ブロックまたは黒ブロックの場合のみ
                //             if (chip.object->type_ == Block::ChipType::White || chip.object->type_ == Block::ChipType::Black) {
                //                 chip.isAnimating = true;
                //                 chip.animState = MapChip::AnimationState::Shrinking;
                //                 chip.animationTime = 0.0f;
                //                 // ここでは type は変更せず、後のアニメーション処理で見た目のみ切り替える
                //                 chip.isColorChangeAnimation = true;
                //             }
                //         }
                //     }
                // }

                for (int x = 0; x < static_cast<int>(mapWidth); ++x) {
                    float delayTime = x * 0.1f; // x座標ごとに遅延を増やす
                    for (int y = 0; y < static_cast<int>(mapHeight); ++y) {
                        MapChip &currentChip = mapChips_[y][x];
                        if (!currentChip.isAnimating) {
                            // 白ブロックまたは黒ブロックの場合のみ
                            if (currentChip.object->type_ == Block::ChipType::White || currentChip.object->type_ == Block::ChipType::Black) {
                                currentChip.isAnimating = true;
                                currentChip.animState = MapChip::AnimationState::Shrinking;
                                currentChip.animationTime = 0.0f;
                                // ここでは type は変更せず、後のアニメーション処理で見た目のみ切り替える
                                currentChip.isColorChangeAnimation = true;

                                currentChip.isDelaying = true;
                                currentChip.delayTime = (x + y) * 0.1f;
                            }
                        }
                    }
                }

                return true; // プレイヤー色反転ブロックが見つかったことを示す
            }
        }
    }

    return false;
}

bool MapChipField::IsAnyChipAnimating() const {
    for (const auto &row : mapChips_) {
        for (const auto &chip : row) {
            if (chip.isAnimating) {
                return true;
            }
        }
    }
    return false;
}
