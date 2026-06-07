#include "FSMComponent.h"

#include "DecisionMaking/GameAIController.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"

// Sets default values for this component's properties
UFsmComponent::UFsmComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// TODO Setup FSM
	Instance = std::make_unique<GameAI::FSM::FSM>();
}

void UFsmComponent::AddState(std::unique_ptr<GameAI::FSM::FState>&& NewState, bool IsInitial) const
{
	Instance->AddState(std::move(NewState), IsInitial);
}

void UFsmComponent::AddTransition(GameAI::FSM::FState* From, GameAI::FSM::FState* To, std::function<bool()> EvalFunc) const
{
	Instance->AddTransition(From, {To, EvalFunc});
}

// Called when the game starts
void UFsmComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UFsmComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// TODO
	if (ASteeringAgent* Agent = Cast<ASteeringAgent>(GetOwner()); Agent)
		Instance->Update(Agent);
}

void UFsmComponent::StartLogic()
{
	Super::StartLogic();

	if (auto Agent = Cast<ASteeringAgent>(GetOwner()); Agent)
		Instance->StartLogic(Agent);
}

void UFsmComponent::StopLogic(const FString& Reason)
{
	Super::StopLogic(Reason);
	
	if (ASteeringAgent* Agent = Cast<ASteeringAgent>(GetOwner()); Agent)
		Instance->StopLogic(Agent);
}

bool UFsmComponent::IsRunning() const
{
	return bIsRunning;
}
