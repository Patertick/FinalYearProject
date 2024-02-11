// Fill out your copyright notice in the Description page of Project Settings.


#include "MemoryBrain.h"
#include "NPC.h"

// Sets default values for this component's properties
UMemoryBrain::UMemoryBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMemoryBrain::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMemoryBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UMemoryBrain::UpdateObjectsInMemory(TArray<AActor*> actorsInView)
{
	for (AActor* actor : actorsInView)
	{
		bool newActor{ false };
		// first check to see if this actor already has a snapshot
		for (int i = 0; i < m_ObjectsInMemory.Num(); i++)
		{
			if (m_ObjectsInMemory[i].actorName == actor->GetName())
			{
				// this actor is in the memory array, update the snapshot
				m_ObjectsInMemory[i].location = actor->GetActorLocation(); // copy the location
				newActor = true; 
			}
		}
		// actor is not in memory, add a new addition
		if (!newActor)
		{
			ActorSnapShot newSnapshot;
			newSnapshot.actorName = actor->GetName();
			newSnapshot.location = actor->GetActorLocation();
			m_ObjectsInMemory.Add(newSnapshot);

			// set material to some arbritrary material that denotes being seen

			if (Cast<ATile3D>(actor) != nullptr)
			{
				Cast<ATile3D>(actor)->m_IsSeen = true;
			}
			else if (Cast<AInteractable>(actor) != nullptr)
			{
				Cast<AInteractable>(actor)->m_IsSeen = true;
			}
			else if (Cast<ANPC>(actor) != nullptr)
			{
				Cast<ANPC>(actor)->m_IsSeen = true;
			}
		}
	}
	
}
