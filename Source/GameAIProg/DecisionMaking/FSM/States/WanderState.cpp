#include "WanderState.h"

#include "Movement/SteeringBehaviors/SteeringAgent.h"

void GameAI::FSM::WanderState::EnterState(TObjectPtr<ASteeringAgent> Agent)
{
	UE_LOG(LogTemp, Warning, TEXT("WanderState::EnterState"));
	Agent->SetSteeringBehavior(WanderBehavior.get());
}

void GameAI::FSM::WanderState::ExitState(TObjectPtr<ASteeringAgent> Agent)
{
	Agent->SetSteeringBehavior(nullptr);
}
