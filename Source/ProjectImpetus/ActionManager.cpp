// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionManager.h"

// Sets default values for this component's properties
UActionManager::UActionManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UActionManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// generate mobility action
	m_MobilityAction = GenerateMobilityAction();

	// generate offensive action
	m_OffensiveAction = GenerateOffensiveAction();

	// generate utility action
	m_UtilityAction = GenerateUtilityAction();

}


// Called every frame
void UActionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

MobilityAction UActionManager::GenerateMobilityAction()
{
	MobilityAction generatedAction;
	generatedAction.speed = FMath::FRandRange(-1.0f, 100.0f);
	generatedAction.isTargetable = FMath::RandBool();
	generatedAction.rangeOfMovement = FMath::FRandRange(generatedAction.speed, generatedAction.speed * 20); // 1 step to 20 steps
	generatedAction.canChangeTrajectory = FMath::RandBool();
	if (!generatedAction.isTargetable) generatedAction.canOtherActionsBeUsed = false;
	else generatedAction.canOtherActionsBeUsed = FMath::RandBool();
	generatedAction.maxStaminaCost = FMath::FRandRange(10.0f, 40.0f);

	return generatedAction;
}

OffensiveAction UActionManager::GenerateOffensiveAction()
{
	OffensiveAction generatedAction;
	generatedAction.numberOfTargetableTiles = FMath::RandRange(1, 30);
	generatedAction.stunTargets = FMath::RandBool();
	generatedAction.pushDistance = FMath::FRandRange(0.0f, 50.0f);
	generatedAction.attackCooldown = FMath::FRandRange(0.5f, 3.0f);

	return generatedAction;
}

UtilityAction UActionManager::GenerateUtilityAction()
{
	float cooldown = FMath::FRandRange(2.0f, 10.0f);
	UtilityType type = static_cast<UtilityType>(FMath::RandRange(0, 3));

	if (type == UtilityType::AllyNPCs)
	{
		UtilityActionAllyNPCs generatedAction;
		generatedAction.utilityType = type;
		generatedAction.utilityCooldown = cooldown;

		generatedAction.canSpeedUpNPCs = FMath::RandBool();
		if (generatedAction.canSpeedUpNPCs) 
		{ 
			generatedAction.canHealNPCs = false; 
			generatedAction.speedUpDuration = FMath::FRandRange(0.5f, 30.0f);
			generatedAction.speedUpPercentage = FMath::FRandRange(25.0f, 50.0f);
		}
		else 
		{
			generatedAction.canHealNPCs = FMath::RandBool();
			if (generatedAction.canHealNPCs)
			{
				generatedAction.healAmount = FMath::FRandRange(5.0f, 20.0f);
			}
		}

		generatedAction.removesStun = FMath::RandBool();
		generatedAction.removesBlindness = FMath::RandBool();
		generatedAction.removesDeafness = FMath::RandBool();

		return generatedAction;
	}
	else if (type == UtilityType::EnemyNPCs)
	{
		UtilityActionEnemyNPCs generatedAction;
		generatedAction.utilityType = type;
		generatedAction.utilityCooldown = cooldown;

		generatedAction.canSlowDownNPCs = FMath::RandBool();
		if (generatedAction.canSlowDownNPCs)
		{
			generatedAction.slowDownDuration = FMath::FRandRange(0.5f, 30.0f);
			generatedAction.slowDownPercentage = FMath::FRandRange(25.0f, 50.0f);
		}
		generatedAction.canStun = FMath::RandBool();
		if (generatedAction.canStun)
		{
			generatedAction.stunDuration = FMath::FRandRange(0.2f, 2.0f);
		}
		generatedAction.canChangeTeam = FMath::RandBool();
		if (generatedAction.canChangeTeam)
		{
			generatedAction.teamChangeDuration = FMath::FRandRange(3.0f, 100.0f);
		}

		generatedAction.canBlind = FMath::RandBool();
		generatedAction.canDeafen = FMath::RandBool();

		return generatedAction;
	}
	else if (type == UtilityType::Tiles)
	{
		UtilityActionTiles generatedAction;
		generatedAction.utilityType = type;
		generatedAction.utilityCooldown = cooldown;

		generatedAction.canChangeTileWalkableToNonWalkable = FMath::RandBool();
		generatedAction.canChangeTileNonWalkableToWalkable = FMath::RandBool();
		generatedAction.canRepairTiles = FMath::RandBool();
		if (generatedAction.canRepairTiles)
		{
			generatedAction.tileRepairTimeTaken = FMath::FRandRange(4.0f, 45.0f);
		}
		generatedAction.canChangeTileRoomType = FMath::RandBool();
		generatedAction.canMakeTileHazardous = FMath::RandBool();
		if (generatedAction.canMakeTileHazardous)
		{
			int32 randomAffect = FMath::RandRange(0, 2);
			if (randomAffect == 0)
			{
				generatedAction.slowNPCPercentage = FMath::FRandRange(30.0f, 60.0f);
				generatedAction.cooldownOfAffect = 0.0f;
			}
			else if (randomAffect == 1)
			{
				generatedAction.damageToDealNPC = FMath::FRandRange(2.0f, 15.0f);
				generatedAction.cooldownOfAffect = FMath::FRandRange(0.5, 2.0f);
			}
			else
			{
				generatedAction.stunNPCDuration = FMath::FRandRange(10.0f, 30.0f);
				generatedAction.cooldownOfAffect = FMath::FRandRange(generatedAction.stunNPCDuration + 2.0f, generatedAction.stunNPCDuration + 5.0f);
			}
			
		}

		return generatedAction;
	}
	else
	{
		UtilityActionObjects generatedAction;
		generatedAction.utilityType = type;
		generatedAction.utilityCooldown = cooldown;
		return generatedAction;
	}

	return UtilityAction();
}

