
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

SteeringOutput BlendedSteering::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedOutput{};

	for (auto& Behaviour : WeightedBehaviors)
	{
		Behaviour.pBehavior->SetTarget(Target);
		
		SteeringOutput Steering{Behaviour.pBehavior->CalculateSteering(DeltaT, Agent)};
		BlendedOutput.LinearVelocity += Steering.LinearVelocity.GetSafeNormal() * Behaviour.Weight;
		BlendedOutput.AngularVelocity += Steering.AngularVelocity * Behaviour.Weight;
	}
	
	return BlendedOutput;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (const auto& Behavior : PriorityBehaviors)
	{
		Behavior->SetTarget(Target);
		Steering = Behavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If none of the behavior return a valid output, last behavior is returned
	return Steering;
}
