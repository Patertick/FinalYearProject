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

	m_PresidingEmotion = Emotion::NoEmotion;
	// ...
}
// Called when the game starts
void UEmotionBrain::BeginPlay()
{
	Super::BeginPlay();

	// create dictionary from default values
	TPair<Emotion, float> newItem;
	newItem.Key = Emotion::Scared;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
	newItem.Key = Emotion::Angry;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
	newItem.Key = Emotion::Disgusted;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
	newItem.Key = Emotion::Relaxed;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
	newItem.Key = Emotion::Joyful;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
	newItem.Key = Emotion::Saddened;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
	newItem.Key = Emotion::Bored;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
	newItem.Key = Emotion::NoEmotion;
	newItem.Value = 0.5f;
	m_EmotionWeights.Add(newItem);
}


// Called every frame
void UEmotionBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// while messages exist, pop message
	if (m_Message.Num() > 0)
	{
		PopMessage();
		// find emotion with most weight and priority
		m_PresidingEmotion = FindPresidingEmotion();
	}

	// ...
}


void UEmotionBrain::PopMessage()
{
	// add to emotion weights dictionary then remove from message array
	TPair<Emotion, float> poppedPair = m_Message[0];
	m_EmotionWeights[poppedPair.Key] *= poppedPair.Value;
	if (m_EmotionWeights[poppedPair.Key] == 0.0f) m_EmotionWeights[poppedPair.Key] = 0.1f; // cap min value at 0.1, otherwise emotions will drop to 0 and be unusable
	if (m_EmotionWeights[poppedPair.Key] > 1.0f) m_EmotionWeights[poppedPair.Key] = 1.0f; // cap max value at 1.0, otherwise a certain emotion may get too high and overwrite every other emotion
	m_Message.RemoveAt(0);
}

void UEmotionBrain::PushMessage(Emotion emotion, float percentage)
{
	if (percentage < 0.0f || percentage > 2.0f) return; // do not add to message if out of range, assert a value between 0.0 and 2.0 (between 100% decrease and 100% increase)

	if (percentage == 1.0f) return; // pointless message as it changes nothing

	// TO DO: add evaluation of quality contradiction E.G. Fear increases by 50% but quality Fearless is active for this NPC

	TPair<Emotion, float> newPair;
	newPair.Key = emotion;
	newPair.Value = percentage;
	m_Message.Add(newPair);
}

Emotion UEmotionBrain::FindPresidingEmotion()
{
	TPair<Emotion, float> highestWeight;
	highestWeight.Key = Emotion::Scared;
	highestWeight.Value = m_EmotionWeights[highestWeight.Key];
	for (const TPair<Emotion, float>& entry : m_EmotionWeights)
	{

		// find a pair with a lower weight than current entry
		if(highestWeight.Value < entry.Value) // overwrite
		{
			highestWeight.Value = entry.Value;
			highestWeight.Key = entry.Key;
		}
		else if (highestWeight.Value == entry.Value) // find which emotion has higher priority
		{
			if (FindPriority(highestWeight.Key) < FindPriority(entry.Key))
			{
				highestWeight.Value = entry.Value;
				highestWeight.Key = entry.Key;
			}
		}
		
	}

	return highestWeight.Key;
}

float UEmotionBrain::FindPriority(Emotion emotion)
{
	// priority values not important, just so long as precedence order is maintained
	switch (emotion)
	{
	case Emotion::Scared:
		return 1.0f; // highest priority, 1
		break;
	case Emotion::Angry:
		return 0.9f; // 2
		break;
	case Emotion::Disgusted:
		return 0.4f; // 5
		break;
	case Emotion::Relaxed:
		return 0.25f; // low priority, 7
		break;
	case Emotion::Joyful:
		return 0.5f; // medium priority, 4
		break;
	case Emotion::Saddened:
		return 0.8f; // 3
		break;
	case Emotion::Bored:
		return 0.3f; // 6
		break;
	case Emotion::NoEmotion:
	default:
		return 0.1f; // lowest priority, 8
		break;

	}
}