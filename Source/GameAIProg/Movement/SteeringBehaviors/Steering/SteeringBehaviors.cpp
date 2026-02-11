#include "SteeringBehaviors.h"

#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();

	const FVector2D DebugTarget = Agent.GetPosition() + Steering.LinearVelocity / Steering.LinearVelocity.Length() * Agent.GetLinearVelocity().Length();
	DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.0f), FVector(DebugTarget, 0.0f), FColor::Magenta);	// will be handy

	return Steering;
}
