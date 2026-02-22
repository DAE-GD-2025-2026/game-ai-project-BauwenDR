#include "Level_CombinedSteering.h"

#include "imgui.h"


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();
	
	auto SeekWeightBehavior{BlendedSteering::WeightedBehavior{new Seek(), 0.5f}};
	auto WanderWeightBehavior{BlendedSteering::WeightedBehavior{new Wander(), 0.5f}};
	BlendedSteeringBehavior = new BlendedSteering({SeekWeightBehavior, WanderWeightBehavior});

	BlendedAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, {0, 0, 0}, FRotator::ZeroRotator);
	BlendedAgent->SetSteeringBehavior(BlendedSteeringBehavior);
	BlendedAgent->SetDebugRenderingEnabled(CanDebugRender);

	PrioritySteeringBehaviour = new PrioritySteering({new Evade(), new Wander()});
	
	PriorityAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, {100, 0, 0}, FRotator::ZeroRotator);
	PriorityAgent->SetSteeringBehavior(PrioritySteeringBehaviour);
	PriorityAgent->SetDebugRenderingEnabled(CanDebugRender);
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();
}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
			BlendedAgent->SetDebugRenderingEnabled(CanDebugRender);
			PriorityAgent->SetDebugRenderingEnabled(CanDebugRender);
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			BlendedSteeringBehavior->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal) { BlendedSteeringBehavior->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		BlendedSteeringBehavior->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		[this](float InVal) { BlendedSteeringBehavior->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
	
		//End
		ImGui::End();
	}
#pragma endregion

	// BlendedSteeringBehavior->SetTarget(MouseTarget);

	BlendedSteeringBehavior->SetTarget(FTargetData{
		PriorityAgent->GetPosition(),
		PriorityAgent->GetRotation(),
		PriorityAgent->GetLinearVelocity(),
		PriorityAgent->GetAngularVelocity(),
	});
	
	PrioritySteeringBehaviour->SetTarget(FTargetData{
		BlendedAgent->GetPosition(),
		BlendedAgent->GetRotation(),
		BlendedAgent->GetLinearVelocity(),
		BlendedAgent->GetAngularVelocity(),
	});
}
