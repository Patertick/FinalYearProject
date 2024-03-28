// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionManager.generated.h"

UENUM()
enum UtilityType {
	AllyNPCs UMETA(DisplayName = "Ally NPCs"),
	EnemyNPCs UMETA(DisplayName = "Enemy NPCs"),
	Tiles UMETA(DisplayName = "Tiles"),
	Objects UMETA(DisplayName = "Objects"),
};

struct MobilityAction {
	float speed; // speed of translation (can be infinite, if so mark as -1)
	bool isTargetable; // is an NPC targetable during this action?
	float rangeOfMovement; // max distance that can be travelled at once (can be infinite, if so mark as -1)
	bool canChangeTrajectory; // can direction of movement be changed during this action?
	bool canOtherActionsBeUsed; // can other actions be used during this action?
	float maxStaminaCost; // stamina cost if this move action is used to its fullest extent

	void operator=(MobilityAction other)
	{
		speed = other.speed;
		isTargetable = other.isTargetable;
		rangeOfMovement = other.rangeOfMovement;
		canChangeTrajectory = other.canChangeTrajectory;
		canOtherActionsBeUsed = other.canOtherActionsBeUsed;
		maxStaminaCost = other.maxStaminaCost;
	}
};

struct OffensiveAction {
	int32 numberOfTargetableTiles; // number of tiles affected
	bool stunTargets; // are targets hit by this attack stunned?
	float pushDistance; // distance to push hit targets back by
	float attackCooldown; // cooldown in seconds

	void operator=(OffensiveAction other)
	{
		numberOfTargetableTiles = other.numberOfTargetableTiles;
		stunTargets = other.stunTargets;
		pushDistance = other.pushDistance;
		attackCooldown = other.attackCooldown;
	}
};

struct UtilityAction {
	float utilityCooldown; // cooldown in seconds
	UtilityType utilityType;
};

struct UtilityActionTiles : UtilityAction {
	bool canChangeTileWalkableToNonWalkable;
	bool canChangeTileNonWalkableToWalkable;
	bool canRepairTiles;
	float tileRepairTimeTaken{ 0.0f }; // in seconds, if canRepair tiles is false, this can be left at 0
	bool canChangeTileRoomType; // can change tile room classifications (from reception to break room)
	bool canMakeTileHazardous; // can put a trap on a tile

	// hazardous properties, leave at default values if not applicable
	float slowNPCPercentage{ 0.0f };
	float damageToDealNPC{ 0.0f };
	float stunNPCDuration{ 0.0f }; // in seconds
	float cooldownOfAffect{ 0.0f };

	void operator=(UtilityActionTiles other)
	{
		utilityCooldown = other.utilityCooldown;
		utilityType = other.utilityType;

		canChangeTileWalkableToNonWalkable = other.canChangeTileWalkableToNonWalkable;
		canChangeTileNonWalkableToWalkable = other.canChangeTileNonWalkableToWalkable;
		canRepairTiles = other.canRepairTiles;
		tileRepairTimeTaken = other.tileRepairTimeTaken;
		canChangeTileRoomType = other.canChangeTileRoomType;
		canMakeTileHazardous = other.canMakeTileHazardous;

		slowNPCPercentage = other.slowNPCPercentage;
		damageToDealNPC = other.damageToDealNPC;
		stunNPCDuration = other.stunNPCDuration;
		cooldownOfAffect = other.cooldownOfAffect;
	}
};

struct UtilityActionObjects : UtilityAction {
	// for now this can be blank
};

struct UtilityActionAllyNPCs : UtilityAction {
	bool canSpeedUpNPCs;
	float speedUpPercentage{ 0.0f };
	float speedUpDuration{ 0.0f }; // in seconds
	bool canHealNPCs;
	float healAmount{ 0.0f };
	bool removesStun;
	bool removesBlindness;
	bool removesDeafness;

	void operator=(UtilityActionAllyNPCs other)
	{
		canSpeedUpNPCs = other.canSpeedUpNPCs;
		speedUpPercentage = other.speedUpPercentage;
		speedUpDuration = other.speedUpDuration;
		canHealNPCs = other.canHealNPCs;
		healAmount = other.healAmount;
		removesStun = other.removesStun;
		removesBlindness = other.removesBlindness;
		removesDeafness = other.removesDeafness;
	}
};

struct UtilityActionEnemyNPCs : UtilityAction {
	bool canSlowDownNPCs;
	float slowDownPercentage{ 0.0f };
	float slowDownDuration{ 0.0f }; // in seconds
	bool canStun;
	float stunDuration{ 0.0f }; // in seconds
	bool canBlind;
	bool canDeafen;
	bool canChangeTeam; // changes the enemy to an ally for a set amount of time
	float teamChangeDuration{ 0.0f }; // in seconds

	void operator=(UtilityActionEnemyNPCs other)
	{
		canSlowDownNPCs = other.canSlowDownNPCs;
		slowDownPercentage = other.slowDownPercentage;
		slowDownDuration = other.slowDownDuration;
		canStun = other.canStun;
		stunDuration = other.stunDuration;
		canBlind = other.canBlind;
		canDeafen = other.canDeafen;
		canChangeTeam = other.canChangeTeam;
		teamChangeDuration = other.teamChangeDuration;
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTIMPETUS_API UActionManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	MobilityAction m_MobilityAction;
	OffensiveAction m_OffensiveAction;
	UtilityAction m_UtilityAction;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	MobilityAction GenerateMobilityAction();
	OffensiveAction GenerateOffensiveAction();
	UtilityAction GenerateUtilityAction();
};
