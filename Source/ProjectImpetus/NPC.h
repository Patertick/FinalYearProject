// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Tile3D.h"
#include "Interactable.h"
#include "SensorBrain.h"
#include "MemoryBrain.h"
#include "PlanningBrain.h"
#include "NPC.generated.h"


UENUM()
enum Directive
{
	MoveHere     UMETA(DisplayName = "MoveHere"),
	FollowThis      UMETA(DisplayName = "FollowThis"),
	AttackThis   UMETA(DisplayName = "AttackThis"),
	InteractThis   UMETA(DisplayName = "InteractThis"),
	DoNothing   UMETA(DisplayName = "DoNothing"),
};

UENUM()
enum Function
{
	MoveFunction,
	AttackFunction,
	InteractFunction,
	NullAction,
};

UENUM()
enum ActionState
{
	Attacking,
	Interacting,
	DoingNothing,
};



struct State {
	ATile3D* tile; // tile the NPC is standing on
	ActionState actionState; // current NPC action state (what is being done)

	bool operator==(State other)
	{
		if (tile == other.tile && actionState == other.actionState)
		{
			return true;
		}
		return false;
	}
};

struct Action
{
	State startingState;
	State endState;
	Function actionType;
};


UCLASS()
class PROJECTIMPETUS_API ANPC : public ACharacter
{
	GENERATED_BODY()


		// define lambda function type
		typedef void(ANPC::* Functions)(void);
public:

	// Sets default values for this character's properties
	ANPC();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Identifiers)
		int32 ID {
		0
	};

	// perception properties, as in what is this NPC perceived to be
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PerceptionProperties)
		bool m_Threat{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float m_DeltaTime{ 0.0f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
		float m_HalfHeight{ 88.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool m_Blind{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool m_IsSeen{ false };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// action functions
	void Move(State startState, State endState);
	void MovePath(Path path);
	void Attack(State startState, State endState);
	void Interact(State startState, State endState);

private:
	Directive m_Directive{ Directive::DoNothing };
	bool m_Controllable{ false };

	// sensory variables

	float m_FieldOfView{ 90.0f };

	float m_MaxViewDistance{ 1000.0f };

	// movement variables

	const float KTILEMAXDISTANCE{ 150.0f }; // max distance between tiles (100x100x100)

	float m_WalkSpeed{ 2.0f };

	Path m_CurrentPath;

	int32 m_PointOnPath{ 0 };

	float m_FollowRange{ 300.0f };

	// attacking variables

	AActor* m_Focus{ nullptr }; // can be an NPC or interactable object, whatever the NPC is currently focusing on

	AActor* m_AttackingEnemy{ nullptr };

	float m_Damage{ 10.0f };

	float m_AttackRange{ KTILEMAXDISTANCE * 2 };

	float m_Health{ 100.0f };
	float m_MaxHealth{ 100.0f };

	// planning variables
	TArray<State> m_GoalStates; // possible states that apply to our goal

	TArray<State> m_PossibleStates; // all possible states for this NPC

	TArray<Action> m_Actions; // collection of possible actions

	State m_InitialState; // starting state; when action is called this is set to state after action

	TArray<ATile3D*> m_MapData; // data of every tile on the map

	Action m_CurrentAction; // current action this NPC is performing


	// brains

	USensorBrain* m_SensorBrain{ nullptr };

	UMemoryBrain* m_MemoryBrain{ nullptr };

	UPlanningBrain* m_PlanningBrain{ nullptr };

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// player control functions
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		void SetDirective(Directive newDirective) { m_Directive = newDirective; }
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		void SetFocus(AActor* focus) { m_Focus = focus; }
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		void MarkAsThreat() { m_Threat = true; }
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		bool IsControllable() { return m_Controllable; }

	// set brains
	UFUNCTION(BlueprintCallable, Category = BrainSet)
		void SetSensorBrain(USensorBrain* sensor) { m_SensorBrain = sensor; }
	UFUNCTION(BlueprintCallable, Category = BrainSet)
		void SetMemoryBrain(UMemoryBrain* memory) { m_MemoryBrain = memory; }
	UFUNCTION(BlueprintCallable, Category = BrainSet)
		void SetPlanningBrain(UPlanningBrain* planning) { m_PlanningBrain = planning; }

	// damage functions
	UFUNCTION(BlueprintCallable, Category = Damage)
		void DealDamage(float damage, AActor* damageInstigator) { 
		m_Health -= damage;
		m_AttackingEnemy = damageInstigator;
	}

	// robustness functions
	UFUNCTION(BlueprintCallable, Category = Robust)
		bool ValidNPC() {
		if (m_SensorBrain == nullptr || m_MemoryBrain == nullptr || m_PlanningBrain == nullptr) // invalid if any agent is null
		{
			return false;
		}
		return true;
	}

	void CallAction(Action action);

	void UpdateMemory(TArray<AActor*> actorsInView) { m_MemoryBrain->UpdateObjectsInMemory(actorsInView); } // call memory brain with new information

	ATile3D* FindClosestTile(FVector2D location);

};


