#pragma once

#include <functional>
#include <memory>

#include "CoreMinimal.h"
#include "BrainComponent.h"
#include "FSM.h"
#include "FSMComponent.generated.h"

namespace GameAI::FSM
{
	class FState;
	class Transition;
}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEAIPROG_API UFsmComponent : public UBrainComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFsmComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void StartLogic() override;
	virtual void StopLogic(const FString& Reason) override;
	
	virtual bool IsRunning() const override; 
	
	void AddState(std::unique_ptr<GameAI::FSM::FState>&& NewState, bool IsInitial = false) const;
	void AddTransition(GameAI::FSM::FState* From, GameAI::FSM::FState* To, std::function<bool()> EvalFunc) const;
		
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	std::unique_ptr<GameAI::FSM::FSM> Instance{};
	bool bIsRunning{false};
};
