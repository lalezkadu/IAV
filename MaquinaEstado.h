#pragma once
#include <vector>
#include "SteeringOutput.h"
#include "state.h"

class MaquinaEstado {
public:
	std::vector<state> states;
	state initialState;
	static MaquinaEstado* MaquinaEstado::create();
	SteeringOutput update();
};
