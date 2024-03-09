// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalCreator.h"

// Sets default values for this component's properties
UGoalCreator::UGoalCreator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGoalCreator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoalCreator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if (m_NPCRef != nullptr)
	{
		m_GoalState = CreateGoal();
		m_NPCRef->CallSetGoal(m_GoalState);
	}
}

