#pragma once
#include "State.h"

class StateMachine {
public:
	std::vector<State> states;
	State currentState;
	//int update();
	//StateMachine* create();

public:
	StateMachine* StateMachine::create() {
		StateMachine *machine = new (std::nothrow) StateMachine();
		return machine;
	}

	int update() {
		State state;
		State::Transition triggeredTransition;
		
		bool cambio = false;
		for (std::vector<State::Transition>::iterator it = currentState.transitions.begin(); it != currentState.transitions.end(); it++) {
			if ((*it).isTriggered()) {
				triggeredTransition = (*it);
				cambio = true;
				break;
			}				
		}

		if (cambio) {
			state = triggeredTransition.getTargetState();
			currentState = state;

		}
		
		return currentState.getAction();
		


	};
};
