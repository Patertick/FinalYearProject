// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MemoryBrain.generated.h"

UENUM()
enum ObjectType {
	Character,
	Interactable,
	Tile,
	NotAnObject,
};

struct ActorSnapShot {
	// data
	FVector location;
	FString actorName; // use this to cross reference for updates
	int32 lastSeenOrder{ 0 }; // 0 means this snapshot is the latest, m_ObjectsInMemory.Num() - 1 would be the oldest snapshot
	ObjectType objectDesignation;
	AActor* objectRef{ nullptr };
};

UENUM()
enum Quality { // these are generated on start, but they can also be dynamically added to NPC during game
	Fearless UMETA(DisplayName = "Fearless"), // no weight given to actions that decrease health
	Coward UMETA(DisplayName = "Coward"), // favours actions that result in taking less to no damage
	MoralCompass UMETA(DisplayName = "Moral Compass"), // favours actions that result in ally NPCs taking less to no damage
	Evil UMETA(DisplayName = "Evil"), // favours actions that result in ally NPCs taking lots of damage
	Violent UMETA(DisplayName = "Violent"), // favours attacking actions whenever possible
	Pacifist UMETA(DisplayName = "Pacifist"), // cannot use attack actions
	Efficient UMETA(DisplayName = "Efficient"), // favours actions that accomplish the most
	Lazy UMETA(DisplayName = "Lazy"), // lower stamina
	Active UMETA(DisplayName = "Active"), // higher stamina
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
	
	const int32 KNUMBEROFQUALITIES{ 9 };
	const int32 KNUMBEROFSTARTINGQUALITIES{ 5 };
	TArray<TEnumAsByte<Quality>> m_Qualities; // NPC qualities that affect their potential for certain actions & emotional responses

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	void UpdateObjectsInMemory(TArray<AActor*> actorsInView);


	Quality GenerateRandomQuality();
	bool DoesQualityContradict(Quality first, Quality second);

	FVector2D GetLastSeenTileLocation();

	ObjectType FindObjectType(AActor* newActor);

	TArray<ActorSnapShot> GetObjectsInMemory() { return m_ObjectsInMemory; }

	bool IsNPCInMemory();

	void ClearMemory() { m_ObjectsInMemory.Empty(); }


	UFUNCTION(BlueprintCallable, Category = Qualities)
		TArray<TEnumAsByte<Quality>> GetQualities() { return m_Qualities; }
		
};
