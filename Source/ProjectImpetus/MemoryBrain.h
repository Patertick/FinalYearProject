// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MemoryBrain.generated.h"

struct ActorSnapShot {
	// data
	FVector location;
	FString actorName; // use this to cross reference for updates
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTIMPETUS_API UMemoryBrain : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMemoryBrain();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	ANPC* m_NPCRef{ nullptr }; // reference to the object this brain works for
	TArray<ActorSnapShot> m_ObjectsInMemory; // will copy actor pointers into this array

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	void UpdateObjectsInMemory(TArray<AActor*> actorsInView);

		
};
