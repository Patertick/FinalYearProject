// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPCStateManager.generated.h"

UENUM()
enum NPCAction {
	NPCAttackUp,
	NPCAttackLeft,
	NPCAttackRight,
	NPCAttackDown,
	NPCAbilityUp,
	NPCAbilityLeft,
	NPCAbilityRight,
	NPCAbilityDown,
	NPCInteractUp,
	NPCInteractLeft,
	NPCInteractRight,
	NPCInteractDown,
	NPCDoingNothing,
	NoNPC,
};

struct TileState {
	NPCAction tileNPCAction; // what is currently happening on this tile
	bool isWalkable; // can this tile be traversed
	int32 NPCIndex; // which NPC this refers to
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

	TArray<TileState> m_MapState; // information holding the current world state

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddToMapState(int32 newNPCIndex);

};
