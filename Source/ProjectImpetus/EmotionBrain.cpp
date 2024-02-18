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

	TMap<Stimuli, EmotionalResponse> generatedDict;
	// create temporary response, null response for detecting invalid responses generated
	EmotionalResponse nullResponse;
	nullResponse.action = GoalAction::NoGoal;
	nullResponse.emotion = Emotion::NoEmotion;
	nullResponse.weight = -1.0f;

	generatedDict.Add(Stimuli::KnownEntitySeen);
	generatedDict.Add(Stimuli::KnownEntityHeard);
	generatedDict.Add(Stimuli::UnknownEntitySeen);
	generatedDict.Add(Stimuli::UnknownEntityHeard);
	generatedDict.Add(Stimuli::Damaged);
	generatedDict.Add(Stimuli::CloseToDeath);
	generatedDict.Add(Stimuli::Dismembered);
	generatedDict.Add(Stimuli::Alone);
	generatedDict.Add(Stimuli::InAGroup);
	generatedDict[Stimuli::KnownEntitySeen] = nullResponse;
	generatedDict[Stimuli::KnownEntityHeard] = nullResponse;
	generatedDict[Stimuli::UnknownEntitySeen] = nullResponse;
	generatedDict[Stimuli::UnknownEntityHeard] = nullResponse;
	generatedDict[Stimuli::Damaged] = nullResponse;
	generatedDict[Stimuli::CloseToDeath] = nullResponse;
	generatedDict[Stimuli::Dismembered] = nullResponse;
	generatedDict[Stimuli::Alone] = nullResponse;
	generatedDict[Stimuli::InAGroup] = nullResponse;
	m_StimuliEmotionDictionary = generatedDict;

	while (count <= m_SearchThreshold) // is the number of searches required reached?
	{
		EmotionalResponse generatedResponse = GenerateResponse();
		generatedDict[Stimuli::KnownEntitySeen] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::KnownEntityHeard] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::UnknownEntitySeen] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::UnknownEntityHeard] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::Damaged] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::CloseToDeath] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::Dismembered] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::Alone] = generatedResponse;
		generatedResponse = GenerateResponse();
		generatedDict[Stimuli::InAGroup] = generatedResponse;
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

EmotionalResponse UEmotionBrain::GenerateResponse()
{
	EmotionalResponse generatedResponse;
	bool validValueGenerated{ false };
	do {
		generatedResponse.action = SelectRandomAction();
		generatedResponse.emotion = SelectRandomEmotion();
		generatedResponse.weight = FMath::FRandRange(0.0f, 1.0f); // 0.0f will never override. 1.0f will always override

		// check if valid
		// note, nothing is a valid emotional response, it was set in the null response as a template rather than a restriction
		if (generatedResponse.weight <= 1.0f && generatedResponse.weight >= 0.0f)
		{
			validValueGenerated = true;
		}
	} while (validValueGenerated == false); // make sure generated value is valid

	return generatedResponse;
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
	float maxFitness{ 0.0f }; // so we can confine fitness to between 0.0 and 1.0

	// for every item
	for (const TPair<Stimuli, EmotionalResponse>& entry : value)
	{
		totalFitness += ActionToEmotionFitness(entry.Value.action, entry.Value.emotion);
		totalFitness += EmotionToWeightFitness(entry.Value.emotion, entry.Value.weight);
		maxFitness++;
		maxFitness++;

		float averageWeightDifference{ 0.0f };
		bool uniqueResponse{ true };
		for (const TPair<Stimuli, EmotionalResponse>& other : value)
		{
			if (entry.Key != other.Key)
			{
				// check for unique responses
				if (entry.Value.action == other.Value.action || entry.Value.emotion == other.Value.emotion)
				{
					uniqueResponse = false;
				}
			}

			// evaluate average weight difference
			averageWeightDifference += abs(entry.Value.weight - other.Value.weight);
		}

		if (uniqueResponse)
		{
			totalFitness++;
		}

		maxFitness++;
		averageWeightDifference = averageWeightDifference / static_cast<float>(value.Num());
		totalFitness += averageWeightDifference;
		maxFitness++;
	}
	return totalFitness / maxFitness;

}

float UEmotionBrain::ActionToEmotionFitness(GoalAction action, Emotion emotion)
{
	switch (action)
	{
	case GoalAction::Evade:
		if (emotion == Emotion::Scared)
		{
			return 0.8f; // ideal emotion for evade
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Relaxed)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.6f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.4f;
		}
		break;
	case GoalAction::Chase:
		if (emotion == Emotion::Scared)
		{
			return 0.5f;
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.8f; // ideal emotion for chase
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.6f;
		}
		else if (emotion == Emotion::Relaxed)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.4f;
		}
		break;
	case GoalAction::Flee:
		if (emotion == Emotion::Scared)
		{
			return 0.8f; // ideal emotion for flee
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Relaxed)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.6f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.4f;
		}
		break;
	case GoalAction::Pursue:
		if (emotion == Emotion::Scared)
		{
			return 0.5f;
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.8f; // ideal emotion for pursue
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.6f;
		}
		else if (emotion == Emotion::Relaxed)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.4f;
		}
		break;
	case GoalAction::Hide:
		if (emotion == Emotion::Scared)
		{
			return 0.8f; // ideal emotion for hide
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Relaxed)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.6f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.4f;
		}
		break;
	case GoalAction::Fight:
		if (emotion == Emotion::Scared)
		{
			return 0.5f;
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.8f; // ideal emotion for fight
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.6f;
		}
		else if (emotion == Emotion::Relaxed)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.1f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.4f;
		}
		break;
	case GoalAction::Interact:
		if (emotion == Emotion::Scared)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.4f; 
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.5f;
		}
		else if (emotion == Emotion::Relaxed)
		{
			return 0.8f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.6f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.4f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.8f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.9f; // ideal emotion for interact
		}
		break;
	case GoalAction::NoGoal:
	default:
		if (emotion == Emotion::Scared)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Angry)
		{
			return 0.2f;
		}
		else if (emotion == Emotion::Disgusted)
		{
			return 0.2f;
		}
		// possible emotions include positive or neutral ones, more excitable mind sets like scared are unlikely
		else if (emotion == Emotion::Relaxed)
		{
			return 0.8f;
		}
		else if (emotion == Emotion::Joyful)
		{
			return 0.8f;
		}
		else if (emotion == Emotion::Saddened)
		{
			return 0.5f;
		}
		else if (emotion == Emotion::Bored)
		{
			return 0.8f;
		}
		else if (emotion == Emotion::NoEmotion)
		{
			return 0.8f;
		}
		break;
	}

	return 0.0f;
}

float UEmotionBrain::EmotionToWeightFitness(Emotion emotion, float weight)
{
	switch (emotion)
	{
	case Emotion::Scared:
		// excitable emotions are more likely to override current goal
		if (weight < 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
		break;
	case Emotion::Angry:
		if (weight < 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
		break;
	case Emotion::Disgusted:
		// less excitable emotions less likely to override current goal
		if (weight > 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
		break;
	case Emotion::Relaxed:
		if (weight > 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
		break;
	case Emotion::Joyful:
		if (weight < 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
		break;
	case Emotion::Saddened:
		if (weight > 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
		break;
	case Emotion::Bored:
		if (weight > 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
		break;
	case Emotion::NoEmotion:
		if (weight > 0.5f)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}
	default:
		return 0.0f;
		break;
	}
}

FString UEmotionBrain::GetStimuliResponse(Stimuli stimuli)
{
	EmotionalResponse response = m_StimuliEmotionDictionary.FindRef(stimuli);

	FString information = "";

	switch (stimuli)
	{
	case Stimuli::KnownEntitySeen:
		information = "Stimuli: Known entity seen, ";
		break;
	case Stimuli::KnownEntityHeard:
		information = "Stimuli: Known entity heard, ";
		break;
	case Stimuli::UnknownEntitySeen:
		information = "Stimuli: Unknown entity seen, ";
		break;
	case Stimuli::UnknownEntityHeard:
		information = "Stimuli: Unknown entity heard, ";
		break;
	case Stimuli::Damaged:
		information = "Stimuli: Damaged, ";
		break;
	case Stimuli::CloseToDeath:
		information = "Stimuli: Close to death, ";
		break;
	case Stimuli::Dismembered:
		information = "Stimuli: Dismembered, ";
		break;
	case Stimuli::Alone:
		information = "Stimuli: Alone, ";
		break;
	case Stimuli::InAGroup:
		information = "Stimuli: In a group, ";
		break;
	default:
		information = "Stimuli: Error non stimuli, ";
	}

	switch (response.action)
	{
	case GoalAction::Evade:
		information = information + "Action: Evading, ";
		break;
	case GoalAction::Chase:
		information = information + "Action: Chasing, ";
		break;
	case GoalAction::Flee:
		information = information + "Action: Fleeing, ";
		break;
	case GoalAction::Pursue:
		information = information + "Action: Pursuing, ";
		break;
	case GoalAction::Hide:
		information = information + "Action: Hiding, ";
		break;
	case GoalAction::Fight:
		information = information + "Action: Fighting, ";
		break;
	case GoalAction::Interact:
		information = information + "Action: Interacting, ";
		break;
	case GoalAction::NoGoal:
	default:
		information = information + "Action: No action, ";
		break;
	}
	switch (response.emotion)
	{
	case Emotion::Scared:
		information = information + "Emotion: Scared, ";
		break;
	case Emotion::Angry:
		information = information + "Emotion: Angry, ";
		break;
	case Emotion::Disgusted:
		information = information + "Emotion: Disgusted, ";
		break;
	case Emotion::Relaxed:
		information = information + "Emotion: Relaxed, ";
		break;
	case Emotion::Joyful:
		information = information + "Emotion: Joyful, ";
		break;
	case Emotion::Saddened:
		information = information + "Emotion: Saddened, ";
		break;
	case Emotion::Bored:
		information = information + "Emotion: Bored, ";
		break;
	case Emotion::NoEmotion:
	default:
		information = information + "Emotion: No emotion, ";
		break;
	}

	information = information + "Weight: " + FString::SanitizeFloat(response.weight);
	return information;
}