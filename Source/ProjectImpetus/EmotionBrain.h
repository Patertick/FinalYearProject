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
	KnownEntitySeen,
	KnownEntityHeard,
	UnknownEntitySeen,
	UnknownEntityHeard,
	Damaged,
	CloseToDeath,
	Dismembered,
	Alone,
	InAGroup,
};

enum GoalAction;


struct EmotionalResponse {
	Emotion emotion; // emotion enum (fear, anger etc.)
	float weight; // weight of emotional response
	GoalAction action; // action to be carried out if emotional response overrides current action (E.G Run or Attack)
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

	TMap<Stimuli, EmotionalResponse> m_StimuliEmotionDictionary; // generated procedurally

	int32 m_SearchThreshold{ 2000 }; // number of searches to do when generating a dictionary

	const int32 KNUMBEROFEMOTIONS = 8;

	const int32 KNUMBEROFACTIONS = 8;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	// procedural functions

	Emotion SelectRandomEmotion();
	GoalAction SelectRandomAction();
	float FitnessFunction(const TMap<Stimuli, EmotionalResponse>& value);

		
};
