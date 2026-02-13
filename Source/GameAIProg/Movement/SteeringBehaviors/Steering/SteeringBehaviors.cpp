#include "SteeringBehaviors.h"

#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

SteeringOutput ISteeringBehavior::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{this->CalculateSteeringInternal(DeltaT, Agent)};
	this->DrawDebugLines(DeltaT, Agent, Steering);

	return Steering;
}

void ISteeringBehavior::DrawDebugLines(const float DeltaT, const ASteeringAgent &Agent, const SteeringOutput& Steering)
{
	if (Agent.GetDebugRenderingEnabled())
	{
		const FVector2D DebugLinearTarget = Agent.GetPosition() + Steering.LinearVelocity * Agent.GetLinearVelocity().Length() * DeltaT;
		DrawDebugDirectionalArrow(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.1f), FVector(DebugLinearTarget, 0.0f), 5.0f, FColor::Magenta);
		
		const FVector2D DebugAngularTarget = Agent.GetPosition() + Steering.AngularVelocity * Agent.GetLinearVelocity().Length() * DeltaT;
		DrawDebugDirectionalArrow(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.1f), FVector(DebugAngularTarget, 0.0f), 5.0f, FColor::Cyan);
	}
}

//SEEK
//*******
SteeringOutput Seek::CalculateSteeringInternal(const float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();

	return Steering;
}

// Flee
//*******
SteeringOutput Flee::CalculateSteeringInternal(const float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{Seek::CalculateSteeringInternal(DeltaT, Agent)};
	Steering.LinearVelocity = -Steering.LinearVelocity;

	return Steering;
}

// Arrive
//*******
Arrive::Arrive(const ASteeringAgent* Agent)
	: DefaultSpeed(Agent->GetMaxLinearSpeed())
{}

SteeringOutput Arrive::CalculateSteeringInternal(const float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{Seek::CalculateSteeringInternal(DeltaT, Agent)};

	if (const double Distance{Steering.LinearVelocity.Length()}; Distance < 75.0)
	{
		Agent.SetMaxLinearSpeed(0.0f);
	} else if (Distance < 350.0) {
		Agent.SetMaxLinearSpeed(Distance / 350.0 * DefaultSpeed);
	} else
	{
		Agent.SetMaxLinearSpeed(DefaultSpeed);
	}

	return Steering;
}

void Arrive::DrawDebugLines(float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering)
{
	Seek::DrawDebugLines(DeltaT, Agent, Steering);
	
	DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.1f), 75.0f, 12, FColor::Red, false, -1, 0, 0, FVector{0, 1,0}, FVector{1,0,0}, false);
	DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.1f), 350.0f, 12, FColor::Blue, false, -1, 0, 0, FVector{0, 1,0}, FVector{1,0,0}, false);
}

SteeringOutput Face::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	const float CurrentRotation = FMath::DegreesToRadians(Agent.GetRotation());
	Steering.AngularVelocity = FMath::Atan2(Target.Position.Y - Agent.GetPosition().Y, Target.Position.X - Agent.GetPosition().X) - CurrentRotation;

	// Normalize the angular velocity to the range -pi to pi
	if (Steering.AngularVelocity > PI) {
		Steering.AngularVelocity -= 2 * PI;
	} else if (Steering.AngularVelocity < -PI) {
		Steering.AngularVelocity += 2 * PI;
	}

	return Steering;
}
