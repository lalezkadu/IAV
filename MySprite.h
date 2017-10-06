#include "cocos2d.h"
USING_NS_CC;

class MySprite {
public:
	cocos2d::Sprite* sprite;
	Vec3 *velocity;
	float orientation;
	float rotation;
	float acceleration;
	float maxSpeed;
	float maxAcceleration;
	float maxAngular;
	float maxRotation;
public:
	static MySprite* MySprite::create();
};