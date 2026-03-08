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

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, 2000, 2000, NrOfCellsY, NrOfCellsX, FlockSize);
	OldPositions.SetNum(FlockSize);
#else 
	Neighbors.SetNum(FlockSize);
#endif

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
		RandomLocation.Z = 88.0f;

		// Spawn the actor at the random location
		FActorSpawnParameters Params{};
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		auto Agent{pWorld->SpawnActor<ASteeringAgent>(AgentClass, RandomLocation, FRotator::ZeroRotator, Params)};

		if (Agent == nullptr) continue;
		
		Agent->SetDebugRenderingEnabled(false);
		Agent->IsLevelControlled = true;
		Agent->SetSteeringBehavior(pPrioritySteering.get());

#ifdef GAMEAI_USE_SPACE_PARTITIONING
		pPartitionedSpace->AddAgent(Agent);
		OldPositions[Index] = Agent->GetPosition();
#endif
		
		Agents[Index] = Agent;
		++Index;
	}

	if (pAgentToEvade)
	{
		pAgentToEvade->SetSteeringBehavior(pEvadeWader.get());
		pAgentToEvade->PrimaryActorTick.bCanEverTick = false;
		pAgentToEvade->IsLevelControlled = true;
	}
}

Flock::~Flock()
{
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	NrOfNeighbors = 0;
#endif
}

void Flock::Tick(float DeltaTime)
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

	for (int Index{0}; Index < FlockSize; ++Index)
	{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		pPartitionedSpace->UpdateAgentCell(Agents[Index], OldPositions[Index]);
		pPartitionedSpace->RegisterNeighbors(Agents[Index], NeighborhoodRadius);
#else 
		RegisterNeighbors(Agents[Index]);
#endif

		Agents[Index]->Steer(DeltaTime);

#ifdef GAMEAI_USE_SPACE_PARTITIONING
		OldPositions[Index] = Agents[Index]->GetPosition();
#endif
		
	}
	
	if (pAgentToEvade)
	{
		pAgentToEvade->Steer(DeltaTime);
	}
}

void Flock::RenderDebug()
{
	RenderNeighborhood();

	Agents[0]->SetDebugRenderingEnabled(DebugRenderSteering);
	pAgentToEvade->SetDebugRenderingEnabled(DebugRenderSteering);

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	if (DebugRenderPartitions)
	{
		pPartitionedSpace->RenderCells();
	}
#endif
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
		bool DebugSteering{DebugRenderSteering};
		bool DebugNeighbourhood{DebugRenderNeighborhood};
		bool DebugPartition{DebugRenderPartitions};
		
		ImGui::Checkbox("Steering debug", &DebugSteering);
		ImGui::Checkbox("Neighbourhood debug", &DebugNeighbourhood);
		ImGui::Checkbox("Partition debug", &DebugPartition);

		DebugRenderSteering = DebugSteering;
		DebugRenderNeighborhood = DebugNeighbourhood;
		DebugRenderPartitions = DebugPartition;

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
	if (DebugRenderNeighborhood)
	{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		pPartitionedSpace->RegisterNeighbors(Agents[0], NeighborhoodRadius);
#else 
		RegisterNeighbors(Agents[0]);
#endif
		
		auto pos = GetAverageNeighborPos();

		if (pos == FVector2D::ZeroVector)
		{
			pos = Agents[0]->GetPosition();
		}

		DrawDebugCircle(
			pWorld,
			FVector(pos, 10.0f),
			NeighborhoodRadius,
			12,
			FColor::Cyan,
			false,
			-1,
			0,
			0,
			FVector{0, 1, 0},
			FVector{1, 0, 0},
			true
		);
	
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
	auto NeighborAgents{GetNeighbors()};
	FVector2D AvgPosition = FVector2D::ZeroVector;

	if (GetNrOfNeighbors() == 0)
	{
		return AvgPosition;
	}

	for (int Index{0}; Index < GetNrOfNeighbors(); ++Index)
	{
		AvgPosition += NeighborAgents[Index]->GetPosition();
	}
	
	return AvgPosition / GetNrOfNeighbors();
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	auto NeighborAgents{GetNeighbors()};
	FVector2D AvgVelocity = FVector2D::ZeroVector;
	
	if (GetNrOfNeighbors() == 0)
	{
		return AvgVelocity;
	}
	
	for (int Index{0}; Index < GetNrOfNeighbors(); ++Index)
	{
		AvgVelocity += NeighborAgents[Index]->GetLinearVelocity();
	}

	return AvgVelocity / GetNrOfNeighbors();
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	pSeekBehavior->SetTarget(Target);
}
