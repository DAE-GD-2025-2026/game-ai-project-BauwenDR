#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
// COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SetTarget(FTargetData{pFlock->GetAverageNeighborPos()});

	return Seek::CalculateSteeringInternal(DeltaT, Agent);
}

//*********************
// SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	FVector2D NewTarget{};

	for (int Index{0}; Index < pFlock->GetNrOfNeighbors(); ++Index)
	{
		const FVector2D ActorToNeighbor{Agent.GetPosition() - pFlock->GetNeighbors()[Index]->GetPosition()};
		// Inverse proportional (normalised devised by distance)
		NewTarget += ActorToNeighbor / ActorToNeighbor.SquaredLength();
	}

	SetTarget(FTargetData{Agent.GetPosition() + NewTarget});
	return Seek::CalculateSteeringInternal(DeltaT, Agent);
}

//*************************
// VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	FVector DesiredVelocity{};

	for (int Index{0}; Index < pFlock->GetNrOfNeighbors(); ++Index)
	{
		DesiredVelocity += pFlock->GetNeighbors()[Index]->GetVelocity();
	}

	FVector2D VelMatchTarget{Agent.GetPosition() + FVector2D{DesiredVelocity.X, DesiredVelocity.Y}};
	SetTarget(FTargetData{VelMatchTarget});

	return Seek::CalculateSteeringInternal(DeltaT, Agent);
}
