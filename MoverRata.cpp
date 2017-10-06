#include "MoverRata.h"
#include "SteeringOutput.h"
#include <iostream>
#include <stdlib.h>
#include "math.h"
#define COCOS2D_DEBUG 1

USING_NS_CC;
#define RAND_MAX = 1;

class SteeringBehavior {
public:
	virtual SteeringOutput getSteering() = 0;
};

class BlendedSteering : public SteeringBehavior {
public:
	std::vector<std::pair<SteeringBehavior*, float>> dic;
	float maxAcceleration = 30;
	float maxAngular;
	SteeringOutput getSteering() {
		SteeringOutput steering, aux;
		std::vector<std::pair<SteeringBehavior*, float>> ::iterator it;
		steering.angular = 0;
		steering.linear.setZero();
		for (it = dic.begin(); it != dic.end(); it++) {
			aux = (*it).first->getSteering();
			steering.linear += aux.linear * (*it).second;
			steering.angular += aux.angular * (*it).second;
		}

		if (steering.linear.length() > maxAcceleration) {
			steering.linear.normalize();
			steering.linear *= maxAcceleration;
		}
		return steering;
	}
};

class Seek: public SteeringBehavior {
public:
	cocos2d::Sprite* target;
	cocos2d::Sprite* character;
	float maxSpeed;
	float maxAcceleration;
	Vec3 radius = Vec3(1, 1, 0);
public:
	SteeringOutput getSteering() {
		SteeringOutput steering;
		auto targetPosition = target->getPosition3D();
		auto myPosition = character->getPosition3D();
		cocos2d::Vec3 aux;
		aux.x = -myPosition.x + targetPosition.x;
		aux.y = -myPosition.y + targetPosition.y;
		aux.normalize();
		aux *= maxSpeed;
		steering.linear = aux;
		steering.angular = atan2(-aux.x, aux.y);

		return steering;
	}
};

class Arrive: SteeringBehavior {
public:
	cocos2d::Sprite* target;
	cocos2d::Sprite* character;
	float maxSpeed;
	float maxAcceleration;
	Vec3 radius = Vec3(1, 1, 0);
public:
	SteeringOutput getSteering() {
		Seek seek;
		SteeringOutput steering;
		seek.character = character;
		seek.target = target;
		auto targetPosition = target->getPosition();
		auto myPosition = character->getPosition();
		cocos2d::Vec2 aux;
		aux = -myPosition + targetPosition;

		if (aux.length() >= radius.length()) {
			return seek.getSteering();
		}
		return steering;
	}
};

class Flee: public SteeringBehavior {
public:
	cocos2d::Sprite* target;
	cocos2d::Sprite* character;
	float maxSpeed;
	float maxAcceleration;
public:
	SteeringOutput getSteering() {
		Seek seek;
		SteeringOutput steering;
		seek.character = character;
		seek.target = target;
		steering = seek.getSteering();
		steering.linear *= -1;
		return steering;
	}
};

class Wander: public SteeringBehavior {
public:
	cocos2d::Sprite* target;
	cocos2d::Sprite* character;
	float maxSpeed = 40;
	float maxAcceleration = 20;
public:
	SteeringOutput getSteering() {
		SteeringOutput steering;
		steering.linear.x = -sin(character->getRotation()) * maxSpeed;
		steering.linear.y = cos(character->getRotation()) * maxSpeed;
		steering.linear.z = 0;
		float x = rand_0_1();
		float y = rand_0_1();
		steering.angular = (y > 0.5) ? x : -x;
		steering.angular *= 10;
		return steering;
	}
};

class SeekD: public SteeringBehavior {
public:
	MySprite* target;
	MySprite* character;
	float maxSpeed;
	float maxAcceleration;
	float maxPrediciton = 100;
	float radius = 1;
	float slowRadius = 5;
	float timeToTarget = 2;

	SteeringOutput getSteering() {
		SteeringOutput steering;
		steering.linear = -character->sprite->getPosition3D() + target->sprite->getPosition3D();
		//CCLOG("%f %f resta", -character->sprite->getPosition().x + target->sprite->getPosition().x, -character->sprite->getPosition().y + target->sprite->getPosition().y);
		steering.linear.normalize();
		steering.linear *= character->maxAcceleration;
		steering.angular = 0;
		//CCLOG("%f %f steering", steering.linear.x, steering.linear.y);
		return steering;
	}
};

class ArriveD: public SteeringBehavior {
public:
	MySprite* target;
	MySprite* character;
	float maxSpeed;
	float maxAcceleration;
	float maxPrediciton = 100;
	float radius = 10;
	float slowRadius = 5;
	float timeToTarget = 2;
public:
	SteeringOutput getSteering() {
		SteeringOutput steering;
		steering.angular = 0;
		float speed;
		Vec3 direction;
		direction.x = target->sprite->getPosition().x - character->sprite->getPosition().x;
		direction.y = target->sprite->getPosition().y - character->sprite->getPosition().y;

		float distance = direction.length();
		if (distance < radius) {
			return steering;
		}
		speed = maxSpeed;
		if (distance <= slowRadius) {
			speed *= distance / (slowRadius);
		}
		direction.normalize();
		direction *= speed;
		steering.linear = -direction + *character->velocity;
		steering.linear.x /= timeToTarget;
		steering.linear.y /= timeToTarget;

		if (steering.linear.length() > character->maxAcceleration) {
			steering.linear.normalize();
			steering.linear *= character->maxAcceleration;
		}

		return steering;
	}
};

class FleeD: public SteeringBehavior {
public:
	MySprite* target;
	MySprite* character;
	float maxSpeed;
	float maxAcceleration;
	float maxPrediciton = 100;
	float radius = 1;
	float slowRadius = 5;
	float timeToTarget = 2;
public:
	SteeringOutput getSteering() {
		SteeringOutput steering;
		steering.linear = character->sprite->getPosition3D() - target->sprite->getPosition3D();
		steering.linear.normalize();
		steering.linear *= character->maxAcceleration;
		steering.angular = 0;
		return steering;
	}
};

class Pursue: public SteeringBehavior {
public:
	MySprite* target;
	MySprite* character;
	float maxSpeed;
	float maxAcceleration;
	float maxPrediciton = 100;
	float radius = 1;
	float slowRadius = 5;
	float timeToTarget = 2;
public:
	SteeringOutput getSteering() {
		Sprite* newTarget = Sprite::create();
		MySprite* newMS = MySprite::create();
		SeekD seekD;
		seekD.character = character;
		float prediction;
		auto direction = target->sprite->getPosition3D() - character->sprite->getPosition3D();
		auto distance = direction.length();
		auto currSpeed = character->velocity->length();
		if (currSpeed <= distance / maxPrediciton) {
			prediction = maxPrediciton;
		}
		else {
			prediction = distance / currSpeed;
		}
		newTarget->setPosition3D(target->sprite->getPosition3D() + *target->velocity * prediction);
		newMS->sprite = newTarget;
		newMS->velocity = new Vec3();
		newMS->velocity->set(*target->velocity);
		seekD.target = newMS;

		return seekD.getSteering();
	}
};

class Evade: public SteeringBehavior {
public:
	MySprite* target;
	MySprite* character;
	float maxSpeed;
	float maxAcceleration;
	float maxPrediciton = 100;
	float radius = 1;
	float slowRadius = 5;
	float timeToTarget = 2;
public:
	SteeringOutput getSteering() {
		Sprite* newTarget = Sprite::create();
		MySprite* newMS = MySprite::create();
		float prediction;
		auto direction = target->sprite->getPosition3D() - character->sprite->getPosition3D();
		auto distance = direction.length();
		auto currSpeed = character->velocity->length();
		if (currSpeed <= distance / maxPrediciton) {
			prediction = maxPrediciton;
		}
		else {
			prediction = distance / currSpeed;
		}
		newTarget->setPosition3D(target->sprite->getPosition3D() + *target->velocity * prediction);
		newMS->sprite = newTarget;
		newMS->velocity = new Vec3();
		newMS->velocity->set(*target->velocity);
		target = newMS;

		FleeD flee;
		flee.character = character;
		flee.target = target;

		return flee.getSteering();
	}
};

class Align: public SteeringBehavior {
public:
	MySprite* character;
	MySprite* target;
	float targetRadius;
	float slowRadius = 5.0;
	float timeToTarget = 0.1;

	virtual SteeringOutput getSteering() {
		SteeringOutput steering;
		float rotation = target->sprite->getRotation() - character->sprite->getRotation();
		rotation = mapToRange(rotation);
		float rotationSize = abs(rotation);
		float targetRotation;
		if (rotationSize < targetRadius) {
			steering.angular = 0;
			return steering;
		}
		if (rotationSize > slowRadius) {
			targetRotation = character->maxRotation;
		}
		else {
			targetRotation = character->maxRotation * rotationSize / slowRadius;
		}
		if (rotationSize != 0) {
			targetRotation *= rotation / rotationSize;
		}
		steering.angular = target->sprite->getRotation() - character->sprite->getRotation();
		steering.angular /= timeToTarget;

		float angularAcceleration = abs(steering.angular);
		if (angularAcceleration > character->maxAngular) {
			steering.angular /= angularAcceleration;
			steering.angular *= character->maxAngular;
		}

		steering.linear = Vec3(0, 0, 0);
		return steering;
	}

	float mapToRange(float x) {
		if (x == 0) {
			return 0;
		}
		float aux = x*M_PI / 180.0;
		float aux1 = atan2(sin(aux), cos(aux));
		return aux1*180.0 / M_PI;
	}
};

class Separation: public SteeringBehavior {
public:
	MySprite* character;
	std::vector<MySprite*> targets;
	float decayCoefficient;
	float threshold;
	float strength;
public:
	SteeringOutput getSteering() {
		SteeringOutput steering;
		Vec3 direction;
		float distance;
		for (int i = 0; i < targets.size(); i++) {
			direction.x = targets[i]->sprite->getPosition3D().x - character->sprite->getPosition3D().x;
			direction.y = targets[i]->sprite->getPosition3D().y - character->sprite->getPosition3D().y;
			distance = direction.length();

			if (distance < threshold) {
				if (distance != 0) {
					strength = std::min(-(decayCoefficient / (distance*distance)), character->maxAcceleration);
				}
				else {
					strength = -character->maxAcceleration;
				}
			}
			else {
				steering.angular = 0;
				continue;
			}

			direction.normalize();
			if (distance != 0) {
				steering.linear += strength * direction;
			}
			else {
				steering.linear += strength * Vec3(1, 1, 1);
			}
			steering.angular = 0;
		}
		return steering;
	}
};

class CollisionAvoidance: public SteeringBehavior {
public:
	MySprite* character;
	std::vector<MySprite*> targets;
	float maxAcceleration;
	float radiusColl;
public:
	SteeringOutput getSteering() {
		SteeringOutput steering;
		float shortestTime = 100000000;
		float firstMinSeparation = 0.0;
		float firstDistance = 0.0;
		float relativeSpeed;
		float timeToCollision;
		float distance;
		float minSeparation;
		bool eTarget = false;
		Vec3 firstRelativePos = Vec3(0, 0, 0);
		Vec3 firstRelativeVel = Vec3(0, 0, 0);
		Vec3 relativePos;
		Vec3 relativeVel;
		MySprite* firstTarget;
		
		for (int i = 0; i < targets.size(); i++) {
			relativePos.x = targets[i]->sprite->getPosition3D().x - character->sprite->getPosition3D().x;
			relativePos.y = targets[i]->sprite->getPosition3D().y - character->sprite->getPosition3D().y;
			relativeVel.x = targets[i]->velocity->x - character->velocity->x;
			relativeVel.y = targets[i]->velocity->y - character->velocity->y;
			
			relativeSpeed = relativeVel.length();
			if (relativeSpeed == 0) {
				continue;
			}

			timeToCollision = abs(relativePos.dot(relativeVel)) / (relativeSpeed * relativeSpeed);
			distance = relativePos.length();
			minSeparation = distance - relativeSpeed*timeToCollision;
			
			if (minSeparation > 2 * radiusColl) {
				continue;
			}

			if (timeToCollision > 0 && timeToCollision < shortestTime) {
				shortestTime = timeToCollision;
				eTarget = true;
				firstTarget = targets[i];
				firstMinSeparation = minSeparation;
				firstDistance = distance;
				firstRelativePos = relativePos;
				firstRelativeVel = relativeVel;
			}
		}

		if (!eTarget) {
			//CCLOG("NoExiste");
			steering.angular = 0;
			return steering;
		}

		if (firstMinSeparation <= 0 || firstDistance < 2 * radiusColl) {
			relativePos.x = firstTarget->sprite->getPosition3D().x - character->sprite->getPosition3D().x;
			relativePos.y = firstTarget->sprite->getPosition3D().y - character->sprite->getPosition3D().y;
		}
		else {
			relativePos = firstRelativePos + firstRelativeVel * timeToCollision;
		}

		relativePos.normalize();
		steering.linear = -relativePos * character->maxAcceleration;
		steering.angular = 0;
		return steering;	
	}
};

class Face: Align {
public:
	MySprite* target;
	MySprite* character;
public:
	virtual SteeringOutput getSteering() override {
		Vec3 direction;
		MySprite * myTarget;
		Align align;

		myTarget= MySprite::create();
		myTarget->sprite = Sprite::create();
		myTarget->sprite->setPosition3D(target->sprite->getPosition3D());
		myTarget->velocity->set(*target->velocity);
		myTarget->maxAngular = target->maxAngular;
		myTarget->maxRotation = target->maxRotation;

		direction.x = target->sprite->getPosition3D().x - character->sprite->getPosition3D().x;
		direction.y = target->sprite->getPosition3D().y - character->sprite->getPosition3D().y;
		
		align.target = myTarget;
		align.character = character;
		align.targetRadius = 1;
		align.slowRadius = 1;
		
		if (direction.length() == 0) {	
			return align.getSteering();
		}

		myTarget->sprite->setRotation(atan2(direction.x, direction.y) * 180.0/ M_PI);
		align.target = myTarget;
		
		return align.getSteering();
	}
};

class LookWhereYoureGoing: public SteeringBehavior {
public:
	MySprite* target;
	MySprite* character;
public:
	SteeringOutput getSteering() {
		SteeringOutput steering;
		Align align;
		MySprite* myTarget = MySprite::create();
		align.target = myTarget;
		align.character = character;

		if (character->velocity->length() == 0) {
			steering.angular = 0;
			return steering;
		}
		myTarget->sprite = Sprite::create();
		myTarget->sprite->setRotation(atan2(character->velocity->x, character->velocity->y) * 180.0 / M_PI);
		return align.getSteering();

	}
};

Scene* MoverRata::createScene()
{
	auto scene = Scene::create();
	auto layer = MoverRata::create();
	scene->addChild(layer);
	return scene;
}

bool MoverRata::init()
{
	if (!Layer::init()) {
		return false;
	}

	mapa = cocos2d::Sprite::create("newmap.png");
	mapa->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
	mapa->setRotation(0);
	this->addChild(mapa);

	sprite = Sprite::create("perritoa.png");
	myS = MySprite::create();
	myS->velocity = new Vec3(0, 0, 0);
	myS->sprite = sprite;
	myS->acceleration = 20;
	myS->maxAcceleration = 20;
	myS->maxSpeed = 40;
	myS->maxAngular = 0;
	myS->maxRotation = 0;
	myS->sprite->setRotation(100);
	sprite->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
	this->addChild(sprite, 0);

	sprite1 = Sprite::create("ovejaa.png");
	sprite1->setRotation(0);
	myS1 = MySprite::create();
	//myS1->velocity = new Vec3(1, 1, 0);
	myS1->sprite = sprite1;
	myS1->acceleration = 20;
	myS1->maxSpeed = 40;
	myS1->maxAcceleration = 20;
	myS1->maxAngular = 30;
	myS1->maxRotation = 30;
	sprite1->setPosition(this->getContentSize().width / 2.5, this->getContentSize().height / 2);
	this->addChild(sprite1, 0);

	sprite2 = Sprite::create("ovejaa.png");
	sprite2->setRotation(0);
	myS2 = MySprite::create();
	//myS2->velocity = new Vec2(1, 1, 0);
	myS2->sprite = sprite2;
	myS2->acceleration = 20;
	myS2->maxAcceleration = 20;
	myS2->maxSpeed = 40;
	myS2->maxAngular = 30;
	myS2->maxRotation = 30;
	sprite2->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2.5);
	this->addChild(sprite2, 0);

	sprite3 = Sprite::create("ovejaa.png");
	sprite3->setRotation(0);
	myS3 = MySprite::create();
	//myS3->velocity = new Vec3(1, 1, 0);
	myS3->sprite = sprite3;
	myS3->acceleration = 20;
	myS3->maxSpeed = 40;
	myS3->maxAcceleration = 20;
	myS3->maxAngular = 30;
	myS3->maxRotation = 30;
	sprite3->setPosition(this->getContentSize().width / 3, this->getContentSize().height / 3);
	this->addChild(sprite3, 0);

	auto eventListener = EventListenerKeyboard::create();

	eventListener->onKeyPressed = [](EventKeyboard::KeyCode keyCode, Event* event) {
		Vec3 loc = event->getCurrentTarget()->getPosition3D();
		
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		case EventKeyboard::KeyCode::KEY_A:
			event->getCurrentTarget()->setPosition(loc.x -= 30, loc.y);
			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		case EventKeyboard::KeyCode::KEY_D:
			event->getCurrentTarget()->setPosition(loc.x += 30, loc.y);
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		case EventKeyboard::KeyCode::KEY_W:
			event->getCurrentTarget()->setPosition(loc.x, loc.y += 30);
			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		case EventKeyboard::KeyCode::KEY_S:
			event->getCurrentTarget()->setPosition(loc.x, loc.y -= 30);
			break;
		case EventKeyboard::KeyCode::KEY_X:
			if (loc.z <= 0) {
				event->getCurrentTarget()->setPosition3D(Vec3(loc.x, loc.y, loc.z += 150));
				break;
			}
		}
	};
	
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, sprite);
	this->scheduleUpdate();
	return true;
}

void MoverRata::updateKinematic(MySprite* chara, float time, SteeringOutput steer) {
	if (steer.linear.length() > 0) {
		float aux = chara->velocity->x * time, auxy = chara->velocity->y * time;
		chara->velocity->set(steer.linear);
		if (chara->sprite->getPositionX() + aux <= this->getContentSize().width + 5 && chara->sprite->getPositionY() + auxy <= this->getContentSize().height + 5 && \
			chara->sprite->getPositionX() + aux > 0 && chara->sprite->getPositionY() + auxy > 0) {
			chara->sprite->setPosition3D(chara->sprite->getPosition3D() + *chara->velocity * time);
		}
		chara->sprite->setRotation(chara->sprite->getRotation() + steer.angular * 1 * time);
		if (chara->sprite->getPositionZ() > 0) {
			chara->velocity->z -= 10;
		}
		else {
			chara->velocity->z = 0;
		}
	}
}

void MoverRata::updateDynamic(MySprite* chara, float time, SteeringOutput steer) {
	if (steer.linear.length() > 0) {
		float aux = chara->velocity->x * time, auxy = chara->velocity->y * time;
		CCLOG(" hola %f", steer.linear.length() );
		if (chara->sprite->getPositionX() + aux <= this->getContentSize().width + 5 && chara->sprite->getPositionY() + auxy <= this->getContentSize().height + 5 && \
			chara->sprite->getPositionX() + aux > 0 && chara->sprite->getPositionY() + auxy > 0) {

			chara->sprite->setPosition3D(chara->sprite->getPosition3D() + *chara->velocity * time);
		}
		
		chara->velocity->set(steer.linear * time + *chara->velocity);
		chara->rotation += steer.angular * time;
		if (chara->velocity->length() > chara->maxSpeed) {
			chara->velocity->normalize();
			chara->velocity->set(*chara->velocity * chara->maxSpeed);
		}
		
	}
	if (steer.angular != 0) {
		CCLOG(" holas %f", steer.angular);
		chara->sprite->setRotation(time * steer.angular + chara->sprite->getRotation());
	}
	if (chara->sprite->getPositionZ() > 0) {
		chara->velocity->z -= 10;
		chara->sprite->setPositionZ(chara->sprite->getPositionZ() - 3);
	}
	else {
		chara->velocity->z = 0;
		chara->sprite->setPositionZ(0);
	}
}

void MoverRata::update(float delta) {
	/*Pursue pursue;
	Evade evade;
	BlendedSteering blend;
	SteeringOutput salida;
	salida.linear.setZero();
	salida.angular = 0;
	blend.maxAngular = 0;
	std::vector<std::pair< SteeringBehavior*, float >> comportamientos;
	pursue.character = myS1;
	pursue.target = myS;
	evade.character = myS;
	evade.target = myS1;
	
	comportamientos.push_back(std::make_pair<SteeringBehavior*, float>(&evade, 1));
	blend.dic = comportamientos;
	salida = blend.getSteering();
	this->updateDynamic(myS1, delta, pursue.getSteering());
	this->updateDynamic(myS, delta, salida);
	

	Pursue pursue;
	pursue.character = myS3;
	pursue.target = myS;
	this->updateDynamic(myS3, delta, pursue.getSteering());
	
	//LookWhereYoureGoing look;
	//look.target = myS;
	//look.character = myS1;
	//look.character->velocity->set(1, 1, 1);
	
	
	/*ArriveD arrive;
	ArriveD arrive2;
	arrive2.character = myS1;
	arrive2.target = myS;
	arrive.character = myS2;
	arrive.target = myS;
	this->updateDynamic(myS2, delta, arrive.getSteering());
	this->updateDynamic(myS1, delta, arrive2.getSteering());*/
	
	// Flock
	SeekD seek1, seek2, seek3;
	seek1.character = myS1;
	seek2.character = myS2;
	seek3.character = myS3;
	seek1.target = myS;
	seek2.target = myS;
	seek3.target = myS;
	//Ahora hay que hacer que también se muevan hacia el centro de masa
	//del flock (promedio ponderado de las posiciones)
	SeekD centro1, centro2, centro3;
	Vec3 centroMasa = myS1->sprite->getPosition3D() + myS2->sprite->getPosition3D() + myS3->sprite->getPosition3D();
	centroMasa *= 1 / 3;
	MySprite* centro = MySprite::create();
	centro->sprite = Sprite::create();
	centro->sprite->setPosition3D(centroMasa);
	centro1.character = myS1;
	centro2.character = myS2;
	centro3.character = myS3;
	centro1.target = centro;
	centro2.target = centro;
	centro3.target = centro;

	//Ahora falta hacer que mantengan la distancia entre ellos
	Separation sep1, sep2, sep3;
	std::vector<MySprite*> targ1, targ2, targ3;
	sep1.character = myS1;
	sep2.character = myS2;
	sep3.character = myS3;

	targ1.push_back(myS2);
	targ1.push_back(myS3);

	targ2.push_back(myS1);
	targ2.push_back(myS3);

	//targ3.push_back(myS1);
	//targ3.push_back(myS2);
	targ3.push_back(myS);

	sep1.targets = targ1;
	sep2.targets = targ2;
	sep3.targets = targ3;

	//'También que volteen hacia donde van;
	LookWhereYoureGoing look1, look2, look3;
	look1.character = myS1;
	look2.character = myS2;
	look3.character = myS3;

	//Prueba ingenua
	BlendedSteering blend1, blend2, blend3;
	std::vector <std::pair<SteeringBehavior*, float>> algo1, algo2, algo3;
	algo1.push_back(std::make_pair<SteeringBehavior*, float>(&seek1, 2));
	algo2.push_back(std::make_pair<SteeringBehavior*, float>(&seek2, 2));
	algo3.push_back(std::make_pair<SteeringBehavior*, float>(&seek3, 2));

	algo1.push_back(std::make_pair<SteeringBehavior*, float>(&centro1, 1));
	algo2.push_back(std::make_pair<SteeringBehavior*, float>(&centro2, 1));
	algo3.push_back(std::make_pair<SteeringBehavior*, float>(&centro3, 1));

	algo1.push_back(std::make_pair<SteeringBehavior*, float>(&sep1, 1));
	algo2.push_back(std::make_pair<SteeringBehavior*, float>(&sep2, 1));
	algo3.push_back(std::make_pair<SteeringBehavior*, float>(&sep3, 1));

	algo1.push_back(std::make_pair<SteeringBehavior*, float>(&look1, 2));
	algo2.push_back(std::make_pair<SteeringBehavior*, float>(&look2, 2));
	algo3.push_back(std::make_pair<SteeringBehavior*, float>(&look3, 2));

	blend1.dic = algo1;
	blend2.dic = algo2;
	blend3.dic = algo3;

	this->updateDynamic(myS1, delta, blend1.getSteering());
	this->updateDynamic(myS2, delta, blend2.getSteering());
	this->updateDynamic(myS3, delta, blend3.getSteering());
}
