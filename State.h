#pragma once
class State {
public:
	std::function<int()> getAction;
public:
	class Transition {
	public:
		std::function<bool()> isTriggered;
		std::function<State()> getTargetState;
		std::function<int()> getAction;

		Transition* State::Transition::create(){
			State::Transition* transiton = new (std::nothrow) State::Transition();
			return transiton;
		};

	};

public:
	std::vector<Transition> transitions;

	

	State* create() {
		State* auxState = new (std::nothrow) State();
		return auxState;
	}
};