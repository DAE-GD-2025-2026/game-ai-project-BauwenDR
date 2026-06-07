#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "States/State.h"

namespace GameAI::FSM
{
	struct FTransitionValue
	{
		FState *To;
		std::function<bool()> EvalFunc;
	};
	
	class FSM
	{
	public:
		void Update(TObjectPtr<ASteeringAgent> Agent);
		
		void AddState(std::unique_ptr<FState>&& NewState, bool IsInitial);
		void AddTransition(FState* From, FTransitionValue Value);
		
		void StartLogic(TObjectPtr<ASteeringAgent> Agent) const;
		void StopLogic(TObjectPtr<ASteeringAgent> Agent) const;
		
	private:
		std::vector<std::unique_ptr<FState>> States{};
		std::unordered_multimap<FState*, FTransitionValue> Transitions{};
		FState *CurrentState{};
	};
}
