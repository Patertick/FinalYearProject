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

		if (m_NPCRef->CallIsActionQueueEmpty())
		{
			if (m_HasTimerRanOut)
			{
				m_GoalState = CreateGoal();
				m_NPCRef->CallSetGoal(m_GoalState);
				m_HasTimerRanOut = false;
			}
			else
			{
				// start timers, if they run out, create new goal
				if (m_NPCRef->GetAlertness() == AlertLevel::High)
				{
					m_HighPriorityTimer += DeltaTime * 25.0f;
					if (m_HighPriorityTimer >= KMAXHIGHTIMER)
					{
						m_HasTimerRanOut = true;
						m_HighPriorityTimer = 0.0f;
					}
				}
				else if (m_NPCRef->GetAlertness() == AlertLevel::Medium)
				{
					m_MediumPriorityTimer += DeltaTime * 25.0f;
					if (m_MediumPriorityTimer >= KMAXMEDIUMTIMER)
					{
						m_HasTimerRanOut = true;
						m_MediumPriorityTimer = 0.0f;
					}
				}
				else if (m_NPCRef->GetAlertness() == AlertLevel::Low)
				{
					m_LowPriorityTimer += DeltaTime * 25.0f;
					if (m_LowPriorityTimer >= KMAXLOWTIMER)
					{
						m_HasTimerRanOut = true;
						m_LowPriorityTimer = 0.0f;
					}
				}
			}
		}
	}
}

