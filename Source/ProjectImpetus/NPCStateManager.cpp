// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCStateManager.h"
#include <Kismet/GameplayStatics.h>
#include "NPC.h"

// Sets default values
ANPCStateManager::ANPCStateManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANPCStateManager::BeginPlay()
{
	Super::BeginPlay();


	
}

// Called every frame
void ANPCStateManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANPCStateManager::AddToMapState(int32 newNPCIndex, ATile3D* startTile, float startHealth, FVector2D startPos)
{
	NPCState newState;
	newState.npcAction = NPCAction::NPCDoingNothingUp; // doesn't really matter what we start with, it'll be overwitten with append
	newState.NPCIndex = newNPCIndex;
	newState.npcHealth = startHealth;
	newState.position = startPos;
	m_MapState.npcStates.Add(newState);
	m_NumberOfNPC++;
}

void ANPCStateManager::AppendNPCState(int32 npcIndex, ATile3D* newTile, NPCAction newAction, float newHealth, FVector2D position)
{
	for (int i = 0; i < m_MapState.npcStates.Num(); i++)
	{
		if (m_MapState.npcStates[i].NPCIndex == npcIndex)
		{
			m_MapState.npcStates[i].tileRef = newTile;
			m_MapState.npcStates[i].npcAction = newAction;
			m_MapState.npcStates[i].npcHealth = newHealth;
			m_MapState.npcStates[i].position = position;
			return;
		}
	}
}
