#include "Level_FSM.h"

#include "FSMComponent.h"
#include "DecisionMaking/GameAIController.h"
#include "States/WanderState.h"


// Sets default values
ALevel_FSM::ALevel_FSM()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_FSM::BeginPlay()
{
	Super::BeginPlay();

	Agent = GetWorld()->SpawnActor<ASteeringAgent>(
		SteeringAgentClass,
		FVector{0, 0, 90}, FRotator::ZeroRotator);
	Agent->SetDebugRenderingEnabled(false);

	if (AGameAIController* AIController = Cast<AGameAIController>(Agent->GetController()))
	{
		if (UFsmComponent* Fsm = Cast<UFsmComponent>(AIController->GetBrainComponent()))
		{
			Fsm->AddState(std::make_unique<GameAI::FSM::WanderState>(), true);
			AIController->RunFiniteStateMachine();
		}
	}
}

// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
