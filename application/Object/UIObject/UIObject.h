#pragma once

#include "application/Base/BaseObject.h"

class UIObject 
{
public:
	void Init();
	void Update();
	void Draw(const ViewProjection& viewProjection);

private:
	std::unique_ptr<BaseObject> objectBook_; // 本モデル
	std::unique_ptr<BaseObject> objectUI_;  // 栞モデル

	std::unique_ptr<BaseObject> objectL_; // Lスティックモデル
	std::unique_ptr<BaseObject> objectA_; // Aボタンモデル
	std::unique_ptr<BaseObject> objectR_; // RBボタンモデル
};
