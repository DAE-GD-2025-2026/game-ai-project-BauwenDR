#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	Agents.SetNum(FlockSize);

	pBlendedSteering = std::make_unique<BlendedSteering>(BlendedSteering{
		{
			BlendedSteering::WeightedBehavior{pCohesionBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pSeparationBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pVelMatchBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pSeekBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pWanderBehavior.get(), 0.5f},
		},
	});
	pPrioritySteering = std::make_unique<PrioritySteering>(PrioritySteering{{pEvadeBehavior.get(), pBlendedSteering.get()}});

	Neighbors.Reserve(FlockSize);
 // TODO: initialize the flock and the memory pool
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
}

void Flock::Tick(float DeltaTime)
{
	for (const auto &agent : Agents)
	{
		RegisterNeighbors(agent);

		const SteeringOutput Output{pPrioritySteering->CalculateSteering(DeltaTime, *agent)};
		
		agent->AddMovementInput(FVector{Output.LinearVelocity, 0.f});
		
		// TODO: trim the agent to the world
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		ImGui::SliderFloat("");
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;
	for (const auto& agent : Agents)
	{
		if (agent == pAgent) continue;
		
		if ((agent->GetPosition() - pAgent->GetPosition()).SquaredLength() <= NeighborhoodRadius*NeighborhoodRadius)
		{
			Neighbors[NrOfNeighbors] = agent;
			NrOfNeighbors++;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D AvgPosition = FVector2D::ZeroVector;

	if (NrOfNeighbors == 0)
	{
		return AvgPosition;
	}

	for (const auto& Agent : Neighbors)
	{
		AvgPosition += Agent->GetPosition();
	}
	
	return AvgPosition / NrOfNeighbors;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D AvgVelocity = FVector2D::ZeroVector;
	
	if (NrOfNeighbors == 0)
	{
		return AvgVelocity;
	}

	for (const auto& Agent : Neighbors)
	{
		AvgVelocity += Agent->GetLinearVelocity();
	}

	return AvgVelocity / NrOfNeighbors;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
}

