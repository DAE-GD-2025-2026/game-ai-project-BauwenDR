#pragma once
#include <vector>

#include "../Steering/SteeringBehaviors.h"

//****************
//BLENDED STEERING
class BlendedSteering final: public ISteeringBehavior
{
public:
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior{};
		float Weight{0.0f};

		WeightedBehavior(ISteeringBehavior* pBehavior, float Weight) :
			pBehavior(pBehavior),
			Weight(Weight)
		{};
	};

	BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors);

	void AddBehaviour(const WeightedBehavior& WeightedBehavior) { WeightedBehaviors.push_back(WeightedBehavior); }

	float* GetWeight(ISteeringBehavior* const SteeringBehavior);
	
	// returns a reference to the weighted behaviors, can be used to adjust weighting. Is not intended to alter the behaviors themselves.
	std::vector<WeightedBehavior>& GetWeightedBehaviorsRef() { return WeightedBehaviors; }
protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;

private:
	std::vector<WeightedBehavior> WeightedBehaviors = {};
};

//*****************
//PRIORITY STEERING
class PrioritySteering final: public ISteeringBehavior
{
public:
	PrioritySteering(const std::vector<ISteeringBehavior*>& PriorityBehaviors)
		:PriorityBehaviors(PriorityBehaviors) 
	{}

	void AddBehaviour(ISteeringBehavior* const Behavior) { PriorityBehaviors.push_back(Behavior); }
protected:
	virtual SteeringOutput CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent) override;

private:
	std::vector<ISteeringBehavior*> PriorityBehaviors = {};
};