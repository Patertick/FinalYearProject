// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoalCreator.h"
#include "ResearcherGoalCreator.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIMPETUS_API UResearcherGoalCreator : public UGoalCreator
{
	GENERATED_BODY()
	
public:
	State CreateGoal() override final { return State(); }
};
