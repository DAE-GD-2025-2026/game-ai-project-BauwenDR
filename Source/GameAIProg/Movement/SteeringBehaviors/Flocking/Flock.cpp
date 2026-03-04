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
	Neighbors.SetNum(FlockSize);

	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>(600.0f);

	pEvadeWader = std::make_unique<Wander>();
	pBlendedSteering = std::make_unique<BlendedSteering>(BlendedSteering{
		{
			BlendedSteering::WeightedBehavior{pSeparationBehavior.get(), SeparationWeight},
			BlendedSteering::WeightedBehavior{pCohesionBehavior.get(), CohesionWeight},
			BlendedSteering::WeightedBehavior{pVelMatchBehavior.get(), VelMatchWeight},
			BlendedSteering::WeightedBehavior{pSeekBehavior.get(), SeekWeight},
			BlendedSteering::WeightedBehavior{pWanderBehavior.get(), WanderWeight},
		},
	});
	pPrioritySteering = std::make_unique<PrioritySteering>(PrioritySteering{{pEvadeBehavior.get(), pBlendedSteering.get()}});

	FRandomStream RandomStream;
	RandomStream.GenerateNewSeed();
	int Index{0};
	while (Index < FlockSize)
	{
		// Generate a random location within the specified radius
		FVector RandomLocation = RandomStream.VRand() * WorldSize / 2.0f;
		RandomLocation.Z = 0.0f;

		// Spawn the actor at the random location
		FActorSpawnParameters params{};
		params.bNoFail = true;
		params.bDeferConstruction = true;
		auto Agent{pWorld->SpawnActor<ASteeringAgent>(AgentClass, RandomLocation, FRotator::ZeroRotator, params)};

		if (Agent == nullptr) continue;
		
		Agent->SetDebugRenderingEnabled(false);
		
		Agents[Index] = Agent;
		Index++;
	}

}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
}

void Flock::Tick(float DeltaTime)
{
	for (int Index{0}; Index < FlockSize; ++Index)
	{
		RegisterNeighbors(Agents[Index]);

		if (Agents[Index] == nullptr || pBlendedSteering == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Agent was null at index %d"), Index);
			continue;
		}
		
		if (pPrioritySteering == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Blended steering was null at index %d"), Index);
			continue;
		}
		
		const SteeringOutput Output{pPrioritySteering->CalculateSteering(DeltaTime, *Agents[Index])};
		
		Agents[Index]->AddMovementInput(FVector{Output.LinearVelocity, 0.f});
		
		// TODO: trim the agent to the world
	}
	
	if (pAgentToEvade)
	{
		const SteeringOutput Steering{pEvadeWader->CalculateSteering(DeltaTime, *pAgentToEvade)};
		pAgentToEvade->AddMovementInput(FVector{Steering.LinearVelocity, 0.0f});
	}
}

void Flock::RenderDebug()
{
	RegisterNeighbors(Agents[0]);
	auto pos = GetAverageNeighborPos();
	
	DrawDebugCircle(
		pWorld,
		FVector(pos, 10.0f),
		10.0f,
		12,
		FColor::Purple,
		false,
		-1,
		0,
		0,
		FVector{0, 1, 0},
		FVector{1, 0, 0},
		true
	);
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
		
		ImGui::SliderFloat("Separation", &Separation, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("Cohesion", &Cohesion, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("Alignment", &Alignment, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("Seek", &Seek, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("Wander", &Wander, 0.0f, 1.0f, "%.1f");

		UpdateWeights(Separation, Cohesion, Alignment, Seek, Wander);
		
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
	for (int Index{0}; Index < FlockSize; ++Index)
	{
		if (Agents[Index] == pAgent) continue;
		
		if ((Agents[Index]->GetPosition() - pAgent->GetPosition()).SquaredLength() <= NeighborhoodRadius*NeighborhoodRadius)
		{
			Neighbors[NrOfNeighbors] = Agents[Index];
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

	for (int Index{0}; Index < NrOfNeighbors; ++Index)
	{
		AvgPosition += Agents[Index]->GetPosition();
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
	
	for (int Index{0}; Index < NrOfNeighbors; ++Index)
	{
		AvgVelocity += Agents[Index]->GetLinearVelocity();
	}

	return AvgVelocity / NrOfNeighbors;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	if(pAgentToEvade)
	{
		pEvadeBehavior->SetTarget(FTargetData{
			pAgentToEvade->GetPosition(),
			pAgentToEvade->GetRotation(),
			pAgentToEvade->GetLinearVelocity(),
			pAgentToEvade->GetAngularVelocity(),
		});
	}

	pSeekBehavior->SetTarget(Target);
}
