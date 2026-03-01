#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
// COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = pFlock->GetAverageNeighborPos() - Agent.GetPosition();

	return Steering;
}

//*********************
// SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	for (int Index{0}; Index < pFlock->GetNrOfNeighbors(); ++Index)
	{
		const FVector2D ActorToNeighbor{Agent.GetPosition() - pFlock->GetNeighbors()[Index]->GetPosition()};
		// Inverse proportional (normalised devised by distance)
		Steering.LinearVelocity += ActorToNeighbor / ActorToNeighbor.SquaredLength();
	}

	return Steering;
}

//*************************
// VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	FVector DesiredVelocity{};

	for (int Index{0}; Index < pFlock->GetNrOfNeighbors(); ++Index)
	{
		DesiredVelocity += pFlock->GetNeighbors()[Index]->GetVelocity();
	}
	DesiredVelocity /= static_cast<float>(pFlock->GetNrOfNeighbors());

	const FVector CurrentVelocity{Agent.GetVelocity()};
	const FVector LinearVelocity{DesiredVelocity - CurrentVelocity};

	Steering.LinearVelocity = FVector2D{LinearVelocity.X, LinearVelocity.Y};


	return Steering;
}
