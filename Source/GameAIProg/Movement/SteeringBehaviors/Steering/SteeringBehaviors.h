#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent);

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
	
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent & Agent) = 0;
	virtual void DrawDebugLines(float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering);
};

class Seek : public ISteeringBehavior
{
protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;
};

class Flee : public Seek
{
protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;
};

class Arrive : public Seek
{
public:
	Arrive(const ASteeringAgent* Agent);

protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;
	virtual void DrawDebugLines(float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering) override;

private:
	float DefaultSpeed{};
};

class Face : public ISteeringBehavior
{
protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;

private:
	float DefaultSpeed{};
};
