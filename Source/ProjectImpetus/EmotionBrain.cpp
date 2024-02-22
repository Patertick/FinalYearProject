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

	// Generate Stimuli Emotional response dictionary using search based procedural generation

	int32 count{ 0 };

	TMap<Stimuli, Emotion> generatedDict;


	generatedDict.Add(Stimuli::KnownFriendlyEntitySeen);
	generatedDict.Add(Stimuli::KnownFriendlyEntityHeard);
	generatedDict.Add(Stimuli::KnownAggressiveEntitySeen);
	generatedDict.Add(Stimuli::KnownAggressiveEntityHeard);
	generatedDict.Add(Stimuli::UnknownEntitySeen);
	generatedDict.Add(Stimuli::UnknownEntityHeard);
	generatedDict.Add(Stimuli::DeadEntitySeen);
	generatedDict.Add(Stimuli::Damaged);
	generatedDict.Add(Stimuli::CloseToDeath);
	generatedDict.Add(Stimuli::Alone);
	generatedDict.Add(Stimuli::InAGroup);
	generatedDict.Add(Stimuli::NoStimuli);
	generatedDict[Stimuli::KnownFriendlyEntitySeen] = Emotion::NoEmotion;
	generatedDict[Stimuli::KnownFriendlyEntityHeard] = Emotion::NoEmotion;
	generatedDict[Stimuli::KnownAggressiveEntitySeen] = Emotion::NoEmotion;
	generatedDict[Stimuli::KnownAggressiveEntityHeard] = Emotion::NoEmotion;
	generatedDict[Stimuli::UnknownEntitySeen] = Emotion::NoEmotion;
	generatedDict[Stimuli::UnknownEntityHeard] = Emotion::NoEmotion;
	generatedDict[Stimuli::DeadEntitySeen] = Emotion::NoEmotion;
	generatedDict[Stimuli::Damaged] = Emotion::NoEmotion;
	generatedDict[Stimuli::CloseToDeath] = Emotion::NoEmotion;
	generatedDict[Stimuli::Alone] = Emotion::NoEmotion;
	generatedDict[Stimuli::InAGroup] = Emotion::NoEmotion;
	generatedDict[Stimuli::NoStimuli] = Emotion::NoEmotion;
	m_StimuliEmotionDictionary = generatedDict;

	while (count <= m_SearchThreshold) // is the number of searches required reached?
	{
		generatedDict[Stimuli::KnownFriendlyEntitySeen] = SelectRandomEmotion();
		generatedDict[Stimuli::KnownFriendlyEntityHeard] = SelectRandomEmotion();
		generatedDict[Stimuli::KnownAggressiveEntitySeen] = SelectRandomEmotion();
		generatedDict[Stimuli::KnownAggressiveEntityHeard] = SelectRandomEmotion();
		generatedDict[Stimuli::UnknownEntitySeen] = SelectRandomEmotion();
		generatedDict[Stimuli::UnknownEntityHeard] = SelectRandomEmotion();
		generatedDict[Stimuli::DeadEntitySeen] = SelectRandomEmotion();
		generatedDict[Stimuli::Damaged] = SelectRandomEmotion();
		generatedDict[Stimuli::CloseToDeath] = SelectRandomEmotion();
		generatedDict[Stimuli::Alone] = SelectRandomEmotion();
		generatedDict[Stimuli::InAGroup] = SelectRandomEmotion();
		generatedDict[Stimuli::NoStimuli] = SelectRandomEmotion();
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

float UEmotionBrain::FitnessFunction(const TMap<Stimuli, Emotion>& value)
{
	// for each entry, check if the emotional response is reasonable
	// add to a total fitness value for evaluation properties

	float totalFitness{ 0.0f }; // 0.0f means completely unfit
	// since each key value can increment the total fitness two potential times, total fitness is number of items * 2
	float maxFitness{ 0.0f }; // so we can confine fitness to between 0.0 and 1.0

	// for every item
	for (const TPair<Stimuli, Emotion>& entry : value)
	{
		// higher fitness for unique entries, completely random dictionary otherwise
		bool unique{ true };
		for (const TPair<Stimuli, Emotion>& other : value)
		{
			if (entry.Key != other.Key)
			{
				if (entry.Value == other.Value)
				{
					unique = false;
				}
			}
		}

		if (unique)
		{
			totalFitness++;
		}
		maxFitness++;
	}

	return totalFitness / maxFitness;

}

FString UEmotionBrain::GetStimuliResponse(Stimuli stimuli)
{
	Emotion response = m_StimuliEmotionDictionary.FindRef(stimuli);

	FString information = "";

	switch (stimuli)
	{
	case Stimuli::KnownFriendlyEntitySeen:
		information = "I saw a friend and i feel ";
		break;
	case Stimuli::KnownFriendlyEntityHeard:
		information = "I heard a friend and i feel ";
		break;
	case Stimuli::KnownAggressiveEntitySeen:
		information = "I saw an enemy and i feel ";
		break;
	case Stimuli::KnownAggressiveEntityHeard:
		information = "I heard an enemy and i feel ";
		break;
	case Stimuli::UnknownEntitySeen:
		information = "I saw something i've never seen before and i feel ";
		break;
	case Stimuli::UnknownEntityHeard:
		information = "I heard something i've never seen before and i feel ";
		break;
	case Stimuli::DeadEntitySeen:
		information = "I just saw a dead body and i feel ";
		break;
	case Stimuli::Damaged:
		information = "I got hurt and i feel ";
		break;
	case Stimuli::CloseToDeath:
		information = "I'm about to die and i feel ";
		break;
	case Stimuli::Alone:
		information = "I'm alone and i feel ";
		break;
	case Stimuli::InAGroup:
		information = "I'm in a group and i feel ";
		break;
	case Stimuli::NoStimuli:
		information = "Nothing is happening and i feel ";
		break;
	default:
		information = "Error non stimuli, ";
	}

	switch (response)
	{
	case Emotion::Scared:
		information = information + "Scared";
		break;
	case Emotion::Angry:
		information = information + "Angry";
		break;
	case Emotion::Disgusted:
		information = information + "Disgusted";
		break;
	case Emotion::Relaxed:
		information = information + "Relaxed";
		break;
	case Emotion::Joyful:
		information = information + "Joyful";
		break;
	case Emotion::Saddened:
		information = information + "Saddened";
		break;
	case Emotion::Bored:
		information = information + "Bored";
		break;
	case Emotion::NoEmotion:
	default:
		information = information + "No emotion";
		break;
	}

	return information;
}