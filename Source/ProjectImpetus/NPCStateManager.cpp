// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCStateManager.h"

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

void ANPCStateManager::AddToMapState(int32 newNPCIndex)
{
}

