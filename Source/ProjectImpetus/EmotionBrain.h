// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmotionBrain.generated.h"

UENUM()
enum Emotion
{
	Scared,
	Angry,
	Disgusted,
	Relaxed,
	Joyful,
	Saddened,
	Bored,
	NoEmotion,
};

UENUM()
enum Stimuli
{
	KnownFriendlyEntitySeen		UMETA(DisplayName = "Known Friendly Entity Seen"),
	KnownFriendlyEntityHeard	UMETA(DisplayName = "Known Friendly Entity Heard"),
	KnownAggressiveEntitySeen   UMETA(DisplayName = "Known Aggressive Entity Seen"),
	KnownAggressiveEntityHeard  UMETA(DisplayName = "Known Aggressive Entity Heard"),
	UnknownEntitySeen			UMETA(DisplayName = "Unknown Entity Seen"),
	UnknownEntityHeard			UMETA(DisplayName = "Unknown Entity Heard"),
	DeadEntitySeen				UMETA(DisplayName = "Dead Entity Seen"),
	Damaged						UMETA(DisplayName = "Damaged"),
	CloseToDeath				UMETA(DisplayName = "Close To Death"),
	Alone						UMETA(DisplayName = "Alone"),
	InAGroup					UMETA(DisplayName = "In A Group"),
	NoStimuli					UMETA(DisplayName = "No Stimuli"),
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTIMPETUS_API UEmotionBrain : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEmotionBrain();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	ANPC* m_NPCRef{ nullptr };

	// procedural variables

	TMap<Stimuli, Emotion> m_StimuliEmotionDictionary; // generated procedurally

	int32 m_SearchThreshold{ 5000 }; // number of searches to do when generating a dictionary

	const int32 KNUMBEROFEMOTIONS = 8;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	// procedural functions

	Emotion SelectRandomEmotion();
	float FitnessFunction(const TMap<Stimuli, Emotion>& value);

	UFUNCTION(BlueprintCallable, Category = Getter)
		FString GetStimuliResponse(Stimuli stimuli);
		
};
