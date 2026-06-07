#pragma once
#include <memory>

#include "State.h"
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

namespace GameAI::FSM
{
	class WanderState final : public FState
	{
	public:
		virtual void EnterState(TObjectPtr<ASteeringAgent> Agent) override;
		virtual void ExitState(TObjectPtr<ASteeringAgent> Agent) override;
		
		virtual ~WanderState() override = default;

	private:
		std::unique_ptr<Wander> WanderBehavior{std::make_unique<Wander>()};
	};
}
