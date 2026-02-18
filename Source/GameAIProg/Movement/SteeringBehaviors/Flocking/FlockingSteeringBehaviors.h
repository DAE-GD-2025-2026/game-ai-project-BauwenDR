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

//VELOCITY MATCH - FLOCKING
//************************
