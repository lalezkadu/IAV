#pragma once


#include "cocos2d.h"
#include "MySprite.h"
#include "SteeringOutput.h"
#include "Nodo.h"
#include "State.h"
#include "StateMachine.h"

using namespace cocos2d;

class MoverRata : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene();
	virtual bool init() override;
	CREATE_FUNC(MoverRata);
	void update(float) override;
	std::unordered_map<std::string, std::vector<std::string>> adyancencias;
	//class SteeringBehavior;

private:
	cocos2d::Sprite* sprite;
	cocos2d::Sprite* sprite3;
	cocos2d::Sprite* sprite2;
	cocos2d::Sprite* sprite1;
	cocos2d::Sprite* rebaSprite;
	cocos2d::Sprite* mapa;
	MySprite* myS;
	MySprite* myS3;
	MySprite* myS2;
	MySprite* myS1;
	MySprite* rebanio;
	CCTMXTiledMap *_tileMap;
	CCTMXLayer *_background;
	void updateKinematic(MySprite* sprite, float delta, SteeringOutput steer);
	void updateDynamic(MySprite* chara, float time, SteeringOutput steer);
};