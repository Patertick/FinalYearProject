// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmotionBrain.generated.h"

UENUM()
enum Emotion
{
	Scared UMETA(DisplayName = "Scared"),
	Angry UMETA(DisplayName = "Angry"),
	Disgusted UMETA(DisplayName = "Disgusted"),
	Relaxed UMETA(DisplayName = "Relaxed"),
	Joyful UMETA(DisplayName = "Joyful"),
	Saddened UMETA(DisplayName = "Saddened"),
	Bored UMETA(DisplayName = "Bored"),
	NoEmotion UMETA(DisplayName = "No Emotion"),
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

	TEnumAsByte<Emotion> m_PresidingEmotion;

	TMap<Emotion, float> m_EmotionWeights; // emotion weights for use in goal creation

	TArray<TPair<Emotion, float>> m_Message; // messages from sensory & memory brains such as Fear 1.4 which translates to increase fear response by 40% & 0.4 is decrease by 60%

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	void PopMessage();

	void PushMessage(Emotion emotion, float percentage);

	Emotion FindPresidingEmotion();

	float FindPriority(Emotion emotion);

	UFUNCTION(BlueprintCallable, Category = Getter)
		TEnumAsByte<Emotion> GetPresidingEmotion() { return m_PresidingEmotion; }
		
};
