#pragma once

class ASteeringAgent;

namespace GameAI::FSM
{
	class FState
	{
	public:
		virtual void EnterState(TObjectPtr<ASteeringAgent> Agent) = 0;
		virtual void ExitState(TObjectPtr<ASteeringAgent> Agent) = 0;
		
		virtual ~FState() = default;
	};
}
