#include "SceneFactory.h"
#include"TitleScene.h"
#include"GameScene.h"
#include"SelectScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
   // 次のシーンを生成
	BaseScene* newScene = nullptr;

	if (sceneName == "TITLE") {
		newScene = new TitleScene();
	}
	else if (sceneName == "SELECT") {
		newScene = new SelectScene();
	}
	else if (sceneName == "GAME") {
		newScene = new GameScene();
	}
	return newScene;
}
