#pragma once
#ifndef STEERINGOUTPUT_H
#define STEERINGOUTPUT_H

#ifndef MYSPRITE_H
#define MYSPRITE_H

//#include "Stage.h"      //<------------------Don't Do This
class SteeringOutput;
class MySprite;



#include "SteeringOutput.h"

#include <functional>
class State {
public:
	std::function<SteeringOutput(MySprite*, MySprite*)> getAction;
	std::string name;
	MySprite* character;
	MySprite* target;
public:
	class Transition {
	public:
		std::function<bool(MySprite*, MySprite*)> isTriggered;
		std::string targetState;
		std::function<SteeringOutput()> getAction;

		/*Transition* State::Transition::create(){
			State::Transition* transiton = new (std::nothrow) State::Transition();
			return transiton;
		};*/

	};

public:
	std::vector<Transition> transitions;

	

	State* create() {
		State* auxState = new (std::nothrow) State();
		return auxState;
	}
};
#endif //  STEERINGOUTPUT_H
#endif // MYSPRITE_H