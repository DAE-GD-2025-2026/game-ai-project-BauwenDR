
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

	for (const auto& Behaviour : WeightedBehaviors)
	{
		Behaviour.pBehavior->SetTarget(Target);
		
		SteeringOutput Steering{Behaviour.pBehavior->CalculateSteering(DeltaT, Agent)};
		BlendedOutput.LinearVelocity += Steering.LinearVelocity.GetSafeNormal() * Behaviour.Weight;
		BlendedOutput.AngularVelocity += Steering.AngularVelocity * Behaviour.Weight;
	}
	
	return BlendedOutput;
}

void BlendedSteering::DrawDebugLines(float DeltaT, const ASteeringAgent& Agent, const SteeringOutput& Steering)
{
	ISteeringBehavior::DrawDebugLines(DeltaT, Agent, Steering);
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteeringInternal(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If none of the behavior return a valid output, last behavior is returned
	return Steering;
}
