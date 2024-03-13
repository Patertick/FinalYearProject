// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPCStateManager.generated.h"

class ATile3D;

UENUM()
enum NPCAction {
	NPCAttackUp,
	NPCAttackLeft,
	NPCAttackRight,
	NPCAttackDown,
	NPCAttackSelf,
	NPCAbilityUp,
	NPCAbilityLeft,
	NPCAbilityRight,
	NPCAbilityDown,
	NPCAbilitySelf,
	NPCInteractUp,
	NPCInteractLeft,
	NPCInteractRight,
	NPCInteractDown,
	NPCInteractSelf,
	NPCMoveUp,
	NPCMoveLeft,
	NPCMoveRight,
	NPCMoveDown,
	NPCDoingNothingUp,
	NPCDoingNothingLeft,
	NPCDoingNothingRight,
	NPCDoingNothingDown,
	NPCDoingNothingSelf,
};

struct NPCState {
	NPCAction npcAction; // what is currently happening on this tile
	float npcHealth; // how much health does this NPC have
	FVector2D position;
	int32 NPCIndex; // which NPC this refers to
	ATile3D* tileRef; // the tile this NPC is on

	void operator=(NPCState other)
	{
		npcAction = other.npcAction;
		npcHealth = other.npcHealth;
		NPCIndex = other.NPCIndex;
		tileRef = other.tileRef;
	}

	bool operator==(NPCState other)
	{
		if (npcAction == other.npcAction)
		{
			if (NPCIndex == other.NPCIndex)
			{
				if (tileRef == other.tileRef)
				{
					return true;
				}
			}

		}
		return false;
	}
	bool operator!=(NPCState other)
	{
		if (npcAction == other.npcAction)
		{
			if (NPCIndex == other.NPCIndex)
			{
				if (tileRef == other.tileRef)
				{
					return false;
				}
			}
		}
		return true;
	}
};

struct WorldState {
	TArray<NPCState> npcStates;

	void operator=(WorldState other)
	{
		npcStates = other.npcStates;
	}

	bool operator==(WorldState other)
	{
		for (int i = 0; i < other.npcStates.Num(); i++)
		{
			if (npcStates[i] != other.npcStates[i])
			{
				return false;
			}
		}
		return true;
	}
};



UCLASS()
class PROJECTIMPETUS_API ANPCStateManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPCStateManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	WorldState m_MapState; // information holding the current world state
	int32 m_NumberOfNPC{ 0 };

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddToMapState(int32 newNPCIndex, ATile3D* startTile, float startHealth, FVector2D startPos); // Add NPC to map state
	void AppendNPCState(int32 npcIndex, ATile3D* newTile, NPCAction newAction, float newHealth, FVector2D position); // append NPC position in map state
	

	WorldState GetMapState() { return m_MapState; }
	int32 GetNumberOfNPC() { return m_NumberOfNPC; }
};
