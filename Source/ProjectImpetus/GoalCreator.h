// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPC.h"
#include "GoalCreator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTIMPETUS_API UGoalCreator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoalCreator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	ANPC* m_NPCRef{ nullptr };

	State m_GoalState;

	// timer variables
	const float KMAXHIGHTIMER{ 15.0f };
	float m_HighPriorityTimer{ 0.0f };
	const float KMAXMEDIUMTIMER{ 30.0f };
	float m_MediumPriorityTimer{ 0.0f };
	const float KMAXLOWTIMER{ 120.0f };
	float m_LowPriorityTimer{ 0.0f };

	bool m_HasTimerRanOut{ true };

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual State CreateGoal() { return State(); } // virtual function, must be overridden by a child or it returns a null state

	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	
		
};
