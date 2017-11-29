#pragma once
#include "State.h"

class StateMachine {
public:
	std::vector<State> states;
	State currentState;
	MySprite* character;
	MySprite* target;

	//int update();
	//StateMachine* create();

public:
	StateMachine* StateMachine::create() {
		StateMachine *machine = new (std::nothrow) StateMachine();
		return machine;
	}

	SteeringOutput update() {
		State state;
		State::Transition triggeredTransition;
		//CCLOG("%s", currentState.name.c_str());
		bool cambio = false;
		for (std::vector<State::Transition>::iterator it = currentState.transitions.begin(); it != currentState.transitions.end(); it++) {
			if ((*it).isTriggered(currentState.character, currentState.target)) {
				triggeredTransition = (*it);
				
				cambio = true;
				break;
			}
		}

		if (cambio) {
			for (int i = 0; i < states.size(); i++) {
				if (triggeredTransition.targetState.compare(states[i].name) == 0){
					
					currentState = states[i];
					//CCLOG("%s", currentState.name.c_str());
					break;
				}
			}
			
		}
		
		return currentState.getAction(currentState.character, currentState.target);
		


	};
};
