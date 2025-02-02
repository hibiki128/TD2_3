#include "ClearCamera.h"
#include "math/myMath.h" // お使いの数学関数やヘルパー関数のヘッダーファイル
#include <Easing.h>

void ClearCamera::Init(ViewProjection* viewProjection)
{
	viewProjection_ = viewProjection;
	translation_ = viewProjection_->translation_;
	rotation_.y = 0.0f;
	matRot_ = MakeIdentity4x4();
	isActive_ = false;
	isFinish_ = false;
}

void ClearCamera::Update(Vector3 targetPos)
{
	if (isActive_) {
		// ターゲットの位置からカメラの現在位置を取得します
		Vector3 currentPos = translation_;
		Vector3 endPos = { targetPos.x,targetPos.y+1.5f,- 10.0f };
		// イージング関数を使ってターゲットの位置に向かって移動します
		float Tmax = 1.0f; // イージングの最大時間（1.0fで完了）

		if (t <= Tmax) {
			t += 1.0f / 60.0f;
		}
		else {
			t = Tmax;
			isFinish_ = true;
		}

		// イージング関数を使って移動させる
		translation_ = EaseInCubic<Vector3>(currentPos, endPos, t, Tmax);

		// カメラ行列の計算（例示的なサンプルです）
		Matrix4x4 scaleMatrix = MakeScaleMatrix(Vector3(1.0f, 1.0f, 1.0f));
		Matrix4x4 rotateXYZMatrix = matRot_; // 必要に応じてDebugCameraのように回転行列を使用する
		Matrix4x4 translateMatrix = MakeTranslateMatrix(translation_);
		Matrix4x4 cameraMatrix = (scaleMatrix * rotateXYZMatrix) * translateMatrix;

		// ViewProjectionクラスに反映します
		viewProjection_->matWorld_ = cameraMatrix;
		viewProjection_->matView_ = Inverse(cameraMatrix);
		viewProjection_->matProjection_ = MakePerspectiveFovMatrix(45.0f * std::numbers::pi_v<float> / 180.0f,
			float(WinApp::kClientWidth) / float(WinApp::kClientHeight),
			0.1f, 1000.0f);
	}
}