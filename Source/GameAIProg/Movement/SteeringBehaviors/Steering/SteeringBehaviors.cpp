#include "SteeringBehaviors.h"

#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

SteeringOutput ISteeringBehavior::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{this->CalculateSteeringInternal(DeltaT, Agent)};

	if (Agent.GetDebugRenderingEnabled())
	{
		this->DrawDebugLines(DeltaT, Agent, Steering);
	}

	return Steering;
}

void ISteeringBehavior::DrawDebugLines(const float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering)
{
	const FVector2D TargetLocation = Agent.GetPosition() +
		Steering.LinearVelocity.GetSafeNormal() * Agent. GetLinearVelocity().Length() / 2.0f;
	
	const FVector CurrentVelocityLocation = FVector(Agent.GetPosition(), 0.0f) +
		Agent.GetVelocity().GetSafeNormal() * Agent.GetLinearVelocity().Length() / 3.0f;
	
	DrawDebugDirectionalArrow(
		Agent.GetWorld(),
		FVector(Agent.GetPosition(), 0.1f),
		FVector(TargetLocation, 0.0f),
		5.0f,
		FColor::Magenta
	);

	DrawDebugDirectionalArrow(
		Agent.GetWorld(),
		FVector(Agent.GetPosition(), 0.1f),
		CurrentVelocityLocation,
		5.0f,
		FColor::Cyan
	);

	DrawDebugCircle(
		Agent.GetWorld(),
		FVector(Target.Position, 0.1f),
		10.0f,
		12,
		FColor::Red,
		false,
		-1,
		0,
		0,
		FVector{0, 1, 0},
		FVector{1, 0, 0},
		true
	);
}

// Seek
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
	}
	else if (Distance < 350.0)
	{
		Agent.SetMaxLinearSpeed(Distance / 350.0 * DefaultSpeed);
	}
	else
	{
		Agent.SetMaxLinearSpeed(DefaultSpeed);
	}

	return Steering;
}

void Arrive::DrawDebugLines(float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering)
{
	Seek::DrawDebugLines(DeltaT, Agent, Steering);

	DrawDebugCircle(
		Agent.GetWorld(),
		FVector(Agent.GetPosition(), 0.1f),
		75.0f,
		12,
		FColor::Red,
		false,
		-1,
		0,
		0,
		FVector{0, 1, 0},
		FVector{1, 0, 0},
		false
	);
	
	DrawDebugCircle(
		Agent.GetWorld(),
		FVector(Agent.GetPosition(), 0.1f), 
		350.0f,
		12,
		FColor::Blue,
		false,
		-1,
		0,
		0,
		FVector{0, 1, 0},
		FVector{1, 0, 0},
		false
	);
}

// Face
//*******
SteeringOutput Face::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	const float CurrentRotation = FMath::DegreesToRadians(Agent.GetRotation());
	
	Steering.AngularVelocity = FMath::Atan2(
		Target.Position.Y - Agent.GetPosition().Y,
		Target.Position.X - Agent.GetPosition().X
	) - CurrentRotation;

	// Normalize the angular velocity to the range -pi to pi
	if (Steering.AngularVelocity > PI)
	{
		Steering.AngularVelocity -= 2 * PI;
	}
	else if (Steering.AngularVelocity < -PI)
	{
		Steering.AngularVelocity += 2 * PI;
	}

	return Steering;
}

SteeringOutput Pursuit::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	const FVector2D Distance{Target.Position - Agent.GetPosition()};
	const float SeekSpeed = Distance.Length() / Agent.GetMaxLinearSpeed();
	const FVector2D FuturePosition{Target.Position + Target.LinearVelocity * SeekSpeed};

	SetTarget(FTargetData(FuturePosition));

	return Seek::CalculateSteeringInternal(DeltaT, Agent);
}

SteeringOutput Evade::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	const bool IsValid = (Target.Position - Agent.GetPosition()).SquaredLength() <= EvadeRadius*EvadeRadius;
	
	SteeringOutput Steering{Pursuit::CalculateSteeringInternal(DeltaT, Agent)};

	Steering.LinearVelocity = -Steering.LinearVelocity;
	Steering.IsValid = IsValid;

	return Steering;
}

void Evade::DrawDebugLines(float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering)
{
	DrawDebugCircle(
		Agent.GetWorld(),
		FVector(Agent.GetPosition(), 0.1f), 
		EvadeRadius,
		12,
		Steering.IsValid ? FColor::Blue : FColor::Red,
		false,
		-1,
		0,
		0,
		FVector{0, 1, 0},
		FVector{1, 0, 0},
		false
	);
	
	Pursuit::DrawDebugLines(DeltaT, Agent, Steering);
}

// Wander
//*******
SteeringOutput Wander::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	const FVector2D CircleCenter{Agent.GetVelocity().GetSafeNormal2D() * OffsetDistance};
	const FVector2D Displacement{FMath::Cos(WanderAngle) * Radius, FMath::Sin(WanderAngle) * Radius};

	WanderAngle += FMath::FRand() * MaxAngleChange - MaxAngleChange * 0.5f;
	SetTarget(FTargetData(Agent.GetPosition() + CircleCenter + Displacement));
	
	return Seek::CalculateSteeringInternal(DeltaT, Agent);
}

void Wander::DrawDebugLines(float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering)
{
	Seek::DrawDebugLines(DeltaT, Agent, Steering);

	const FVector2D CircleCenter{Agent.GetVelocity().GetSafeNormal2D() * OffsetDistance * 10.0f};
	
	DrawDebugCircle(
		Agent.GetWorld(),
		FVector(Agent.GetPosition() + CircleCenter, 0.1f),
		Radius * 10.0f,
		12,
		FColor::Red,
		false,
		-1,
		0,
		0,
		FVector{0, 1, 0},
		FVector{1, 0, 0}, false);
}
