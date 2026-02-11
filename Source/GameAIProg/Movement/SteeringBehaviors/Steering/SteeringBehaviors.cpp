#include "SteeringBehaviors.h"

#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();

	if (Agent.GetDebugRenderingEnabled())
	{
		// const FVector2D DebugTarget = Agent.GetPosition() + Steering.LinearVelocity / Steering.LinearVelocity.Length() * Agent.GetLinearVelocity().Length() * DeltaT;
		// DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.0f), FVector(DebugTarget, 0.0f), FColor::Magenta);	// will be handy
	}

	return Steering;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{Seek::CalculateSteering(DeltaT, Agent)};
	Steering.LinearVelocity = -Steering.LinearVelocity;

	return Steering;
}

Arrive::Arrive(const ASteeringAgent* Agent)
	: DefaultSpeed(Agent->GetMaxLinearSpeed())
{}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{Seek::CalculateSteering(DeltaT, Agent)};

	double Distance{Steering.LinearVelocity.Length()};
	if (Distance < 75.0)
	{
		Agent.SetMaxLinearSpeed(0.0f);
	} else if (Distance < 350.0) {
		Agent.SetMaxLinearSpeed(Distance / 350.0 * DefaultSpeed);
	} else
	{
		Agent.SetMaxLinearSpeed(DefaultSpeed);
	}

	DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.0f), 75.0f, 12, FColor::Red, false, -1, 0, 0, FVector{0, 1,0}, FVector{1,0,0}, false);
	DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.0f), 350.0f, 12, FColor::Blue, false, -1, 0, 0, FVector{0, 1,0}, FVector{1,0,0}, false);

	return Steering;
}
