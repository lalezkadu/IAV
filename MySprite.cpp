#include "MySprite.h"
#include "cocos2d.h"

USING_NS_CC;

MySprite* MySprite::create() {
	MySprite *auxsprite = new (std::nothrow) MySprite();
	auxsprite->velocity = new Vec3();
	auxsprite->maquina = new StateMachine();
	return auxsprite;
}