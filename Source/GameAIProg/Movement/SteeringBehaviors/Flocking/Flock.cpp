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
			BlendedSteering::WeightedBehavior{pSeparationBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pCohesionBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pVelMatchBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pSeekBehavior.get(), 0.5f},
			BlendedSteering::WeightedBehavior{pWanderBehavior.get(), 0.5f},
		},
	});
	pPrioritySteering = std::make_unique<PrioritySteering>(PrioritySteering{{pEvadeBehavior.get(), pBlendedSteering.get()}});

	Neighbors.Reserve(FlockSize);

	FRandomStream RandomStream;
	RandomStream.GenerateNewSeed();
	int Index{0};
	while (Index < FlockSize)
	{
		// Generate a random location within the specified radius
		FVector RandomLocation = RandomStream.VRand() * WorldSize / 2.0f;

		// Spawn the actor at the random location
		auto Agent{pWorld->SpawnActor<ASteeringAgent>(AgentClass, RandomLocation, FRotator::ZeroRotator)};

		if (Agent != nullptr) ++Index;
	}
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

		float Cohesion{CohesionWeight};
		float Separation{SeparationWeight};
		float Alignment{VelMatchWeight};
		float Seek{SeekWeight};
		float Wander{WanderWeight};
		
		ImGui::SliderFloat("Separation", &Separation, 0.0f, 1.0f, "0.1f");
		ImGui::SliderFloat("Cohesion", &Cohesion, 0.0f, 1.0f, "0.1f");
		ImGui::SliderFloat("Alignment", &Alignment, 0.0f, 1.0f, "0.1f");
		ImGui::SliderFloat("Seek", &Seek, 0.0f, 1.0f, "0.1f");
		ImGui::SliderFloat("Wander", &Wander, 0.0f, 1.0f, "0.1f");
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

void Flock::UpdateWeights(float Separation, float Cohesion, float Alignment, float Seek, float Wander)
{
	if (
		Separation == SeparationWeight &&
		Cohesion == CohesionWeight &&
		Alignment == VelMatchWeight &&
		Seek == SeekWeight &&
		Wander == WanderWeight
	) {
		return;
	}

	SeparationWeight = Separation;
	CohesionWeight = Cohesion;
	VelMatchWeight = Alignment;
	SeekWeight = Seek;
	WanderWeight = Wander;

	pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = Separation;
	pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = Cohesion;
	pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = Alignment;
	pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = Seek;
	pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = Wander;
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
	pPrioritySteering->SetTarget(Target);
	pSeekBehavior->SetTarget(Target);
}

