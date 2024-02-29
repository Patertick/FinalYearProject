// Fill out your copyright notice in the Description page of Project Settings.


#include "MemoryBrain.h"
#include "Math/UnrealMathUtility.h"
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
	while (m_Qualities.Num() < KNUMBEROFSTARTINGQUALITIES)
	{
		bool contradiction = false;
		Quality newQuality = GenerateRandomQuality();

		for (Quality existingQuality : m_Qualities)
		{
			if (DoesQualityContradict(existingQuality, newQuality)) // checks for repeating & contradicting qualities
			{
				contradiction = true;
			}
		}

		if (!contradiction) // if none of the qualities contradict, add to array
		{
			m_Qualities.Add(newQuality);
		}
	}

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

			//// set material to some arbritrary material that denotes being seen

			//if (Cast<ATile3D>(actor) != nullptr)
			//{
			//	Cast<ATile3D>(actor)->m_IsSeen = true;
			//}
			//else if (Cast<AInteractable>(actor) != nullptr)
			//{
			//	Cast<AInteractable>(actor)->m_IsSeen = true;
			//}
			//else if (Cast<ANPC>(actor) != nullptr)
			//{
			//	Cast<ANPC>(actor)->m_IsSeen = true;
			//}
		}
	}
	
}

Quality UMemoryBrain::GenerateRandomQuality()
{
	int32 randomNum;
	randomNum = FMath::RandRange(0, KNUMBEROFQUALITIES - 1);

	switch (randomNum)
	{
	case 0:
		return Quality::Blind;
		break;
	case 1:
		return Quality::Deaf;
		break;
	case 2:
		return Quality::Fearless;
		break;
	case 3:
		return Quality::Coward;
		break;
	case 4:
		return Quality::MoralCompass;
		break;
	case 5:
		return Quality::Evil;
		break;
	case 6:
		return Quality::Violent;
		break;
	case 7:
		return Quality::Pacifist;
		break;
	case 8:
		return Quality::Efficient;
		break;
	case 9:
		return Quality::Stupid;
		break;
	case 10:
		return Quality::Smart;
		break;
	case 11:
		return Quality::Lazy;
		break;
	case 12:
		return Quality::Active;
		break;
	case 13:
		return Quality::AngerIssues;
		break;
	case 14:
		return Quality::Charismatic;
		break;
	default:
		return Quality::NullQuality;
	}
}

bool UMemoryBrain::DoesQualityContradict(Quality first, Quality second)
{
	switch (first)
	{
	case Quality::Blind:
		if (second == Quality::Blind) // no repeats
		{
			return true;
		}
		return false;
		break;
	case Quality::Deaf:
		if (second == Quality::Deaf)
		{
			return true;
		}
		return false;
		break;
	case Quality::Fearless:
		if (second == Quality::Fearless || second == Quality::Coward) // coward clearly contradicts fearless
		{
			return true;
		}
		return false;
		break;
	case Quality::Coward:
		if (second == Quality::Coward || second == Quality::Fearless)
		{
			return true;
		}
		return false;
		break;
	case Quality::MoralCompass:
		if (second == Quality::MoralCompass || second == Quality::Evil || second == Quality::Violent) // evil & violent contradict moral compass
		{
			return true;
		}
		return false;
		break;
	case Quality::Evil:
		if (second == Quality::Evil || second == Quality::MoralCompass || second == Quality::Pacifist)
		{
			return true;
		}
		return false;
		break;
	case Quality::Violent:
		if (second == Quality::Violent || second == Quality::MoralCompass || second == Quality::Pacifist)
		{
			return true;
		}
		return false;
		break;
	case Quality::Pacifist:
		if (second == Quality::Pacifist || second == Quality::Evil || second == Quality::Violent)
		{
			return true;
		}
		return false;
		break;
	case Quality::Efficient:
		if (second == Quality::Efficient || second == Quality::Lazy || second == Quality::Stupid) // stupid & lazy contradict efficient
		{
			return true;
		}
		return false;
		break;
	case Quality::Stupid:
		if (second == Quality::Stupid || second == Quality::Smart || second == Quality::Efficient) // stupid & smart contradict
		{
			return true;
		}
		return false;
		break;
	case Quality::Smart:
		if (second == Quality::Smart || second == Quality::Stupid)
		{
			return true;
		}
		return false;
		break;
	case Quality::Lazy:
		if (second == Quality::Lazy || second == Quality::Efficient || second == Quality::Active)
		{
			return true;
		}
		return false;
		break;
	case Quality::Active:
		if (second == Quality::Active || second == Quality::Lazy)
		{
			return true;
		}
		return false;
		break;
	case Quality::AngerIssues:
		if (second == Quality::AngerIssues) // nothing contradicts anger issues at the moment
		{
			return true;
		}
		return false;
		break;
	case Quality::Charismatic:
		if (second == Quality::Charismatic) // nothing contradicts charismatic at the moment
		{
			return true;
		}
		return false;
		break;
	default:
		return true;
	}
}