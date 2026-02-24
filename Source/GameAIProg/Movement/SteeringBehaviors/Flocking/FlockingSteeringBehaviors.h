#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek
{
public:
	Cohesion(Flock* const pFlock) :pFlock(pFlock) {};

protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;

private:
	Flock* pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Separation final : public Seek
{
public:
	Separation(Flock* const pFlock) :pFlock(pFlock) {};

protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;

private:
	Flock* pFlock = nullptr;
};

//VELOCITY MATCH - FLOCKING
//************************
class VelocityMatch final : public Seek
{
public:
	VelocityMatch(Flock* const pFlock) :pFlock(pFlock) {};

protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;

private:
	Flock* pFlock = nullptr;
};
