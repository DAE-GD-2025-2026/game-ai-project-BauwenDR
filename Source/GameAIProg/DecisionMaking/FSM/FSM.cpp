#include "FSM.h"

#include "Movement/SteeringBehaviors/SteeringAgent.h"

void GameAI::FSM::FSM::Update(TObjectPtr<ASteeringAgent> Agent)
{
	if (!Transitions.contains(CurrentState)) return;
	
	for (auto [Itr, RangeEnd]{Transitions.equal_range(CurrentState)}; Itr != RangeEnd; ++Itr)
	{
		auto const &[To, TransitionValue]{Itr->second};
		
		if (TransitionValue())
		{
			CurrentState->ExitState(Agent);
			To->EnterState(Agent);
		}
	}
}

void GameAI::FSM::FSM::AddState(std::unique_ptr<FState>&& NewState, bool IsInitial)
{
	States.emplace_back(std::move(NewState));
	
	if (IsInitial)
		CurrentState = States.back().get();
}

void GameAI::FSM::FSM::AddTransition(FState* From, FTransitionValue Value)
{
	Transitions.emplace(From, Value);
}

void GameAI::FSM::FSM::StartLogic(TObjectPtr<ASteeringAgent> Agent) const
{
	UE_LOG(LogTemp, Warning, TEXT("FSM::StartLogic"));
	CurrentState->EnterState(Agent);
}

void GameAI::FSM::FSM::StopLogic(TObjectPtr<ASteeringAgent> Agent) const
{
	CurrentState->ExitState(Agent);
}
