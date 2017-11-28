#include "MoverRata.h"
#include "SteeringOutput.h"
#include <iostream>
#include <stdlib.h>
#include "math.h"
#include "Nodo.h"
#include <fstream>
#include <sstream>
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
		//steering.angular = steering.angular > maxAngular ? maxAngular : steering.angular;
		return steering;
	}
};

class Seek : public SteeringBehavior {
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

class Arrive : SteeringBehavior {
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

class Flee : public SteeringBehavior {
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

class Wander : public SteeringBehavior {
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

class SeekD : public SteeringBehavior {
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
		//steering.linear.x = -character->sprite->getPosition().x + target->sprite->getPosition().x;
		//steering.linear.y = -character->sprite->getPosition().y + target->sprite->getPosition().y;
		//CCLOG("%f %f %f", character->sprite->getPosition3D().x, character->sprite->getPosition3D().y, character->sprite->getPosition3D().z);
		//CCLOG("%f %f resta", -character->sprite->getPosition().x + target->sprite->getPosition().x, -character->sprite->getPosition().y + target->sprite->getPosition().y);

		steering.linear.normalize();
		steering.linear *= character->maxAcceleration;
		steering.angular = 0;
		//CCLOG("%f %f steering", steering.linear.x, steering.linear.y);
		return steering;
	}
};

class ArriveD : public SteeringBehavior {
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

class FleeD : public SteeringBehavior {
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

class Pursue : public SteeringBehavior {
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

class Evade : public SteeringBehavior {
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

class Align : public SteeringBehavior {
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

class Separation : public SteeringBehavior {
public:
	MySprite* character;
	std::vector<MySprite*> targets;
	float decayCoefficient = 20000;
	float threshold = 80;
	float strength = 200;
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
				CCLOG("DISTANCA < TRESH");
				if (distance != 0) {
					strength = std::min(-(decayCoefficient / (distance*distance)), character->maxAcceleration);
				}
				else {
					strength = -character->maxAcceleration;
				}
			}
			else {
				CCLOG("ELSE");
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

class CollisionAvoidance : public SteeringBehavior {
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
			steering.angular = 0;
			//CCLOG("VAINA");
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

class Face : Align {
public:
	MySprite* target;
	MySprite* character;
public:
	virtual SteeringOutput getSteering() override {
		Vec3 direction;
		MySprite * myTarget;
		Align align;

		myTarget = MySprite::create();
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

		myTarget->sprite->setRotation(atan2(direction.x, direction.y) * 180.0 / M_PI);
		align.target = myTarget;

		return align.getSteering();
	}
};

class LookWhereYoureGoing : public SteeringBehavior {
public:
	MySprite* target;
	MySprite* character;
public:
	SteeringOutput getSteering() {
		/*SeekD arrive;
		Face face;

		face.target = target;
		face.character = character;

		arrive.target = target;
		arrive.character = character;
		character->maxAngular = 80;

		SteeringOutput faces = face.getSteering();
		SteeringOutput blending = arrive.getSteering();

		blending.angular = faces.angular;
		return blending;*/

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

class PathFollowing : public SteeringBehavior {
public:
	MySprite* character;
	std::vector<Vec2> camino;

	SteeringOutput getSteering() {
		SteeringOutput salida;
		if (!camino.empty()) {
			CCLOG("YEPA");
			Vec2 actual = camino.back();
			ArriveD seek;
			MySprite* aux = MySprite::create();
			aux->sprite = Sprite::create();
			seek.character = character;
			if (actual.distance(character->sprite->getPosition()) < 35) {
				CCLOG("YUPA");
				camino.pop_back();
				if (!camino.empty()) {
					actual = camino.back();
				}
			}
			aux->sprite->setPosition(actual);
			seek.target = aux;
			return seek.getSteering();
				

			}
			else {
				return salida;
			}
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

	std::unordered_map<std::string, std::vector<std::string>> adyancencias;
	adyancencias["53.3333333333,806.0"] = { "26.6666666667,693.333333333","130.0,758.0","26.6666666667,918.666666667" };
	adyancencias["568.0,335.0"] = { "689.0,400.0","436.0,346.333333333","589.333333333,233.666666667" };
	adyancencias["470.666666667,843.333333333"] = { "414.333333333,723.666666667","398.666666667,956.0" };
	adyancencias["168.666666667,223.666666667"] = { "223.0,278.666666667","243.333333333,111.0","57.6666666667,259.0" };
	adyancencias["893.0,758.666666667"] = { "720.333333333,741.333333333","962.333333333,721.333333333","954.666666667,865.333333333" };
	adyancencias["720.333333333,741.333333333"] = { "893.0,758.666666667","643.666666667,795.0","792.333333333,618.666666667" };
	adyancencias["497.0,107.333333333"] = { "385.0,59.3333333333","644.333333333,48.0","485.0,191.666666667" };
	adyancencias["923.666666667,354.0"] = { "995.666666667,423.333333333","862.0,400.0","952.0,204.666666667" };
	adyancencias["132.333333333,51.6666666667"] = { "243.333333333,111.0","29.0,146.333333333" };
	adyancencias["306.0,903.333333333"] = { "196.666666667,918.666666667","398.666666667,956.0","292.0,796.666666667" };
	adyancencias["223.0,278.666666667"] = { "168.666666667,223.666666667","353.0,250.333333333" };
	adyancencias["660.0,633.333333333"] = { "594.333333333,659.666666667","661.333333333,554.0" };
	adyancencias["103.333333333,639.333333333"] = { "130.0,758.0","26.6666666667,693.333333333","155.0,558.333333333" };
	adyancencias["735.666666667,533.333333333"] = { "792.333333333,618.666666667","661.333333333,554.0","689.0,400.0" };
	adyancencias["325.0,729.666666667"] = { "266.0,610.0","292.0,796.666666667","414.333333333,723.666666667" };
	adyancencias["995.666666667,423.333333333"] = { "923.666666667,354.0","967.333333333,522.333333333" };
	adyancencias["130.0,758.0"] = { "103.333333333,639.333333333","180.0,806.0","53.3333333333,806.0" };
	adyancencias["118.666666667,971.333333333"] = { "196.666666667,918.666666667","26.6666666667,918.666666667" };
	adyancencias["196.666666667,918.666666667"] = { "306.0,903.333333333","118.666666667,971.333333333" };
	adyancencias["243.333333333,111.0"] = { "168.666666667,223.666666667","385.0,59.3333333333","132.333333333,51.6666666667" };
	adyancencias["661.333333333,554.0"] = { "623.666666667,501.0","735.666666667,533.333333333","660.0,633.333333333" };
	adyancencias["398.666666667,956.0"] = { "306.0,903.333333333","470.666666667,843.333333333" };
	adyancencias["862.0,400.0"] = { "923.666666667,354.0","764.333333333,352.0","903.0,501.0" };
	adyancencias["689.0,400.0"] = { "568.0,335.0","764.333333333,352.0","623.666666667,501.0","735.666666667,533.333333333" };
	adyancencias["385.0,59.3333333333"] = { "243.333333333,111.0","497.0,107.333333333" };
	adyancencias["754.0,197.333333333"] = { "700.0,250.666666667" };
	adyancencias["436.0,346.333333333"] = { "568.0,335.0","325.333333333,303.666666667","354.333333333,415.666666667" };
	adyancencias["105.666666667,436.333333333"] = { "189.0,379.0","28.6666666667,377.333333333","155.0,558.333333333" };
	adyancencias["871.666666667,954.666666667"] = { "733.333333333,891.666666667","954.666666667,865.333333333" };
	adyancencias["881.333333333,149.333333333"] = { "952.0,204.666666667","870.333333333,48.0" };
	adyancencias["266.0,610.0"] = { "325.0,729.666666667","155.0,558.333333333","382.0,577.666666667" };
	adyancencias["623.666666667,501.0"] = { "661.333333333,554.0","689.0,400.0" };
	adyancencias["632.0,961.0"] = { "733.333333333,891.666666667","542.333333333,854.333333333" };
	adyancencias["353.0,250.333333333"] = { "223.0,278.666666667","485.0,191.666666667","325.333333333,303.666666667" };
	adyancencias["589.333333333,233.666666667"] = { "568.0,335.0","485.0,191.666666667","700.0,250.666666667" };
	adyancencias["180.0,806.0"] = { "130.0,758.0" };
	adyancencias["903.0,501.0"] = { "871.0,580.333333333","862.0,400.0" };
	adyancencias["57.6666666667,259.0"] = { "168.666666667,223.666666667","28.6666666667,377.333333333","29.0,146.333333333" };
	adyancencias["189.0,379.0"] = { "354.333333333,415.666666667","105.666666667,436.333333333" };
	adyancencias["952.0,204.666666667"] = { "923.666666667,354.0","881.333333333,149.333333333" };
	adyancencias["995.666666667,575.333333333"] = { "967.333333333,522.333333333" };
	adyancencias["644.333333333,48.0"] = { "497.0,107.333333333","744.333333333,96.0" };
	adyancencias["485.0,191.666666667"] = { "497.0,107.333333333","589.333333333,233.666666667","353.0,250.333333333" };
	adyancencias["594.333333333,659.666666667"] = { "660.0,633.333333333","472.333333333,652.666666667" };
	adyancencias["700.0,250.666666667"] = { "754.0,197.333333333","589.333333333,233.666666667","764.333333333,352.0" };
	adyancencias["994.333333333,633.333333333"] = { "932.0,659.666666667","995.666666667,575.333333333" };
	adyancencias["870.333333333,48.0"] = { "881.333333333,149.333333333","744.333333333,96.0" };
	adyancencias["28.6666666667,377.333333333"] = { "105.666666667,436.333333333","57.6666666667,259.0" };
	adyancencias["472.333333333,652.666666667"] = { "594.333333333,659.666666667","414.333333333,723.666666667","382.0,577.666666667" };
	adyancencias["29.0,146.333333333"] = { "57.6666666667,259.0","132.333333333,51.6666666667" };
	adyancencias["292.0,796.666666667"] = { "306.0,903.333333333","325.0,729.666666667" };
	adyancencias["155.0,558.333333333"] = { "105.666666667,436.333333333","266.0,610.0","103.333333333,639.333333333" };
	adyancencias["325.333333333,303.666666667"] = { "436.0,346.333333333","353.0,250.333333333" };
	adyancencias["733.333333333,891.666666667"] = { "871.666666667,954.666666667","632.0,961.0","643.666666667,795.0" };
	adyancencias["26.6666666667,918.666666667"] = { "118.666666667,971.333333333","53.3333333333,806.0" };
	adyancencias["962.333333333,721.333333333"] = { "893.0,758.666666667" };
	adyancencias["542.333333333,854.333333333"] = { "632.0,961.0","643.666666667,795.0" };
	adyancencias["792.333333333,618.666666667"] = { "735.666666667,533.333333333","871.0,580.3333333333334","643.666666667,795.0" };
	adyancencias["954.666666667,865.333333333"] = { "893.0,758.666666667","871.666666667,954.666666667" };
	adyancencias["354.333333333,415.666666667"] = { "436.0,346.333333333","189.0,379.0","465.0,505.333333333" };
	adyancencias["871.0,580.333333333"] = { "903.0,501.0","792.333333333,618.666666667","932.0,659.666666667" };
	adyancencias["414.333333333,723.666666667"] = { "470.666666667,843.333333333","325.0,729.666666667","472.333333333,652.666666667" };
	adyancencias["764.333333333,352.0"] = { "862.0,400.0","689.0,400.0","700.0,250.666666667" };
	adyancencias["382.0,577.666666667"] = { "266.0,610.0","472.333333333,652.666666667","465.0,505.333333333" };
	adyancencias["465.0,505.333333333"] = { "354.333333333,415.666666667","382.0,577.666666667" };
	adyancencias["643.666666667,795.0"] = { "720.333333333,741.333333333","542.333333333,854.333333333","733.333333333,891.666666667" };
	adyancencias["26.6666666667,693.333333333"] = { "53.3333333333,806.0","103.333333333,639.333333333" };
	adyancencias["932.0,659.666666667"] = { "994.333333333,633.333333333","871.0,580.333333333" };
	adyancencias["744.333333333,96.0"] = { "644.333333333,48.0","870.333333333,48.0" };
	adyancencias["967.333333333,522.333333333"] = { "995.666666667,423.333333333","995.666666667,575.333333333" };




	

	
	Nodo nodo = Nodo("871.666666667,954.666666667", Vec2(871.666666667, 954.666666667), NULL, 0, 0);
	CCLOG("aaargo %s", nodo.state.c_str());
	nodo.adyacencias = adyancencias;
	Nodo meta = Nodo("26.6666666667,918.666666667", Vec2(26.6666666667, 918.666666667), NULL, 0, 0);
	Nodo solucion = nodo.a_estrella(meta);
	























	//mapa = cocos2d::Sprite::create("newmap.png");
	//mapa->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
	//mapa->setRotation(0);
	//this->addChild(mapa);

	sprite = Sprite::create("perritoa.png");
	myS = MySprite::create();
	//PRUEBA CREAR MÁQUINA DE ESTADOS
	std::vector<State> states;
	State pastando;
	State::Transition prueba2;
	prueba2.isTriggered = []() { return false; };
	std::function<int()> prueba1 = []() { return 1; };
	pastando.getAction = prueba1;
	states.push_back(pastando);

	myS->maquina->states = states;
	myS->maquina->currentState = pastando;


	//PRUEBA CREAR MÁQUINA DE ESTADOS
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
	myS1->maxAngular = 50;
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
	sprite3->setPosition(40, 918.5);
	myS3->camino = solucion.camino1();
	CCLOG("%f esto esta pasando que loco", solucion.g);


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
		//if (abs(chara->sprite->getRotation() - steer.angular) > 0.2) {
		chara->sprite->setRotation(chara->sprite->getRotation() + steer.angular * 1 * time);
		//}
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
		//CCLOG(" hola %f", steer.linear.length() );
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
		//CCLOG(" holas %f", steer.angular);
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

	//#########################FLOCK########################//
	/*
	//Seeks para el flock, el flock se mueve hace el jugador
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

	targ3.push_back(myS1);
	targ3.push_back(myS2);
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
	*/
	//#########################FLOCK########################//

	//#########################ARRIVE########################//
	/*
	ArriveD arrive;
	arrive.character = myS1;
	arrive.target = myS;
	this->updateDynamic(myS1, delta, arrive.getSteering());
	//#########################ARRIVE########################//
	*/
	//#########################Arrive, look where you're going and collision avoidance########################//
	LookWhereYoureGoing look;
	ArriveD arrive;
	CollisionAvoidance avoid;
	BlendedSteering blend;
	std::vector<MySprite*> aux;
	std::vector<std::pair<SteeringBehavior*, float>> mezcla;
	look.character = myS1;
	arrive.character = myS1;
	arrive.target = myS;
	avoid.character = myS1;

	aux.push_back(myS2);
	aux.push_back(myS3);

	avoid.targets = aux;
	avoid.radiusColl = 10;


	mezcla.push_back(std::make_pair<SteeringBehavior*, float>(&arrive, 1));
	mezcla.push_back(std::make_pair<SteeringBehavior*, float>(&avoid, 1));
	mezcla.push_back(std::make_pair<SteeringBehavior*, float>(&look, 1));
	blend.dic = mezcla;

	this->updateDynamic(myS1, delta, blend.getSteering());
	PathFollowing path;
	path.character = myS3;
	path.camino = myS3->camino;
	this->updateDynamic(myS3, delta, path.getSteering());
	//std::vector<int> prueba;
	//prueba.push_back(1);
	//std::vector<int> prueba1 = prueba;
	//prueba1.push_back(1);
	//CCLOG("1 2 %d %d", prueba.size(), prueba1.size());
	//CCLOG(" hola %f %f", myS->sprite->getPosition().x, myS->sprite->getPosition().y );
}