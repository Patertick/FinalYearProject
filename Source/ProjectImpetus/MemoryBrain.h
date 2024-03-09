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
	Blind UMETA(DisplayName = "Blind"), // unable to perceive through sight sensors
	Deaf UMETA(DisplayName = "Deaf"), // unable to perceive through hearing sensors
	Fearless UMETA(DisplayName = "Fearless"), // fear responses are decreased by 100%
	Coward UMETA(DisplayName = "Coward"), // fear responses are increased by 50%
	MoralCompass UMETA(DisplayName = "Moral Compass"), // 'good' actions have increased chance
	Evil UMETA(DisplayName = "Evil"), // 'bad' actions have increased chance
	Violent UMETA(DisplayName = "Violent"), // attacking actions have increased chance
	Pacifist UMETA(DisplayName = "Pacifist"), // attacking actions have decreased chance
	Efficient UMETA(DisplayName = "Efficient"), // rational and efficient actions have increased chance
	Stupid UMETA(DisplayName = "Stupid"), // more likely to prefer emotional responses
	Smart UMETA(DisplayName = "Smart"), // more likely to prefer rational responses
	Lazy UMETA(DisplayName = "Lazy"), // decreased chance to movement actions
	Active UMETA(DisplayName = "Active"), // increased chance to movement actions
	AngerIssues UMETA(DisplayName = "Anger Issues"), // anger responses are increased by 50%
	Charismatic UMETA(DisplayName = "Charismatic"), // increases likability, increases leading ability
	NullQuality UMETA(DisplayName = "No Quality"), // for error checking
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
	
	const int32 KNUMBEROFQUALITIES{ 15 };
	const int32 KNUMBEROFSTARTINGQUALITIES{ 4 };
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


	UFUNCTION(BlueprintCallable, Category = Qualities)
		TArray<TEnumAsByte<Quality>> GetQualities() { return m_Qualities; }
		
};
