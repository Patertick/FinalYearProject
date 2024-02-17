// Fill out your copyright notice in the Description page of Project Settings.


#include "EmotionBrain.h"
#include "NPC.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UEmotionBrain::UEmotionBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEmotionBrain::BeginPlay()
{
	Super::BeginPlay();

	// ...

	// Generate Stimuli Emotional response dictionary using search based procedural generation

	int32 count{ 0 };

	TMap<Stimuli, EmotionalResponse> generatedDict;
	// create temporary response, null response for detecting invalid responses generated
	EmotionalResponse nullResponse;
	nullResponse.action = GoalAction::NoGoal;
	nullResponse.emotion = Emotion::NoEmotion;
	nullResponse.weight = -1.0f;

	generatedDict[Stimuli::KnownEntitySeen] = nullResponse;
	generatedDict[Stimuli::KnownEntityHeard] = nullResponse;
	generatedDict[Stimuli::UnknownEntitySeen] = nullResponse;
	generatedDict[Stimuli::UnknownEntityHeard] = nullResponse;
	generatedDict[Stimuli::Damaged] = nullResponse;
	generatedDict[Stimuli::CloseToDeath] = nullResponse;
	generatedDict[Stimuli::Dismembered] = nullResponse;
	generatedDict[Stimuli::Alone] = nullResponse;
	generatedDict[Stimuli::InAGroup] = nullResponse;

	while (count <= m_SearchThreshold) // is the number of searches required reached?
	{
		for (TPair<Stimuli, EmotionalResponse>& entry : generatedDict)
		{
			bool validValueGenerated{ false };
			do {
				entry.Value.action = SelectRandomAction();
				entry.Value.emotion = SelectRandomEmotion();
				entry.Value.weight = FMath::FRandRange(0.0f, 1.0f); // 0.0f will never override. 1.0f will always override

				// check if valid
				// note, nothing is a valid emotional response, it was set in the null response as a template rather than a restriction
				if (entry.Value.weight <= 1.0f && entry.Value.weight >= 0.0f)
				{
					validValueGenerated = true;
				}
			} while (validValueGenerated == false); // make sure generated value is valid
		}

		if (FitnessFunction(generatedDict) > FitnessFunction(m_StimuliEmotionDictionary))
		{
			m_StimuliEmotionDictionary = generatedDict;
		}

		count++;
	}
	
}


// Called every frame
void UEmotionBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

Emotion UEmotionBrain::SelectRandomEmotion()
{
	int32 randomNum;
	randomNum = FMath::RandRange(0, KNUMBEROFEMOTIONS - 1);
	switch (randomNum)
	{
	case 0:
		return Emotion::Scared;
	case 1:
		return Emotion::Angry;
	case 2:
		return Emotion::Disgusted;
	case 3:
		return Emotion::Relaxed;
	case 4:
		return Emotion::Joyful;
	case 5:
		return Emotion::Saddened;
	case 6:
		return Emotion::Bored;
	case 7:
	default:
		return Emotion::NoEmotion;
	}
}

GoalAction UEmotionBrain::SelectRandomAction()
{
	int32 randomNum;
	randomNum = FMath::RandRange(0, KNUMBEROFACTIONS - 1);
	switch (randomNum)
	{
	case 0:
		return GoalAction::Evade;
	case 1:
		return GoalAction::Chase;
	case 2:
		return GoalAction::Flee;
	case 3:
		return GoalAction::Pursue;
	case 4:
		return GoalAction::Hide;
	case 5:
		return GoalAction::Fight;
	case 6:
		return GoalAction::Interact;
	case 7:
	default:
		return GoalAction::NoGoal;
	}
}

float UEmotionBrain::FitnessFunction(const TMap<Stimuli, EmotionalResponse>& value)
{
	// for each entry, check if the emotional response is reasonable
	// add to a total fitness value for evaluation properties

	float totalFitness{ 0.0f }; // 0.0f means completely unfit
	// since each key value can increment the total fitness two potential times, total fitness is number of items * 2
	float maxFitness = static_cast<float>(value.Num() * 2); // so we can confine fitness to between 0.0 and 1.0
	TArray<EmotionalResponse> responses;

	for (const TPair<Stimuli, EmotionalResponse>& entry : value)
	{
		bool isUnique{ true };
		for (int i = 0; i < responses.Num(); i++)
		{
			if (entry.Value.emotion == responses[i].emotion)
			{
				isUnique = false;
			}
		}
		if (isUnique)
		{
			totalFitness++;
		}
		if (entry.Value.weight >= 0.0f && entry.Value.weight <= 1.0f)
		{
			totalFitness++;
		}
	}

	return totalFitness / maxFitness;

}

