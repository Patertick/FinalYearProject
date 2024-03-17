// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Tile3D.h"
#include "Interactable.h"
#include "SensorBrain.h"
#include "MemoryBrain.h"
#include "PlanningBrain.h"
#include "EmotionBrain.h"
#include "NPC.generated.h"

UENUM()
enum CharacterType {
	ResearcherCharacter		UMETA(DisplayName = "Researcher"),
	AgentCharacter			UMETA(DisplayName = "Agent"),
	VolunteerCharacter		UMETA(DisplayName = "Volunteer"),
	OtherCharacter			UMETA(DisplayName = "Other"),
};

UENUM()
enum AlertLevel {
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High"),
};

UENUM()
enum Directive
{
	MoveHere	 UMETA(DisplayName = "MoveHere"),
	FollowThis   UMETA(DisplayName = "FollowThis"),
	AttackThis   UMETA(DisplayName = "AttackThis"),
	InteractThis UMETA(DisplayName = "InteractThis"),
	DoNothing    UMETA(DisplayName = "DoNothing"),
};



UENUM()
enum GoalAction // for use in planning brain and emotion brain
{
	Evade,
	Chase,
	Flee,
	Pursue,
	Hide,
	Fight,
	Interact,
	NoGoal,

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool m_RunQLearning{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CharacterType)
		TEnumAsByte<CharacterType> m_CharacterType{ CharacterType::OtherCharacter };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// action functions
	State Move(State startState, State endState);
	//bool Move(const Path& path, int32& pointOnPath);
	State Attack(State startState, State endState);
	State Interact(State startState, State endState);
	State Ability(State startState, State endState);

private:
	Directive m_Directive{ Directive::DoNothing };
	bool m_Controllable{ false };

	// self properties

	FString m_Name{ "" };

	const int32 KNAMEGENERATIONTHRESHOLD{ 200 };

	const int32 KNAMELENGTHMIN{ 1 };

	const int32 KNAMELENGTHMAX{ 5 };

	int32 m_Index{ 0 };

	bool m_HasEscaped{ false };
	bool m_HasDied{ false };

	ATile3D* m_StartTile{ nullptr };


	bool m_EndOfScenario{ false };

	// sensory variables

	float m_FieldOfView{ 90.0f };

	float m_MaxViewDistance{ 1000.0f };

	// movement variables

	const float KTILEMAXDISTANCE{ 150.0f }; // max distance between tiles (100x100x100)

	float m_WalkSpeed{ 2.0f };

	float m_FollowRange{ KTILEMAXDISTANCE * 2 };

	// attacking variables

	AActor* m_AttackingEnemy{ nullptr };

	float m_Damage{ 10.0f };

	float m_AttackRange{ KTILEMAXDISTANCE * 2 };

	float m_Health{ 100.0f };
	float m_MaxHealth{ 100.0f };

	float m_DamageDealt{ 0.0f };
	float m_LastDamageDealt{ 0.0f };

	// acting

	Action m_CurrentAction; // current action this NPC is performing

	// NPCs current alert level (affected by processed sensory & memory functions)

	AlertLevel m_Alertness{ AlertLevel::Low };


	// brains

	USensorBrain* m_SensorBrain{ nullptr };

	UMemoryBrain* m_MemoryBrain{ nullptr };

	UPlanningBrain* m_PlanningBrain{ nullptr };

	UEmotionBrain* m_EmotionBrain{ nullptr };

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// player control functions
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		void SetDirective(Directive newDirective) { m_Directive = newDirective; }
	Directive GetDirective() { return m_Directive; }
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		void SetFocus(AActor* focus) { m_PlanningBrain->SetFocus(focus); }
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
	UFUNCTION(BlueprintCallable, Category = BrainSet)
		void SetEmotionBrain(UEmotionBrain* emotion) { m_EmotionBrain = emotion; }

	UFUNCTION(BlueprintCallable, Category = Run)
		void RunQLearning() { m_RunQLearning = true; }
	UFUNCTION(BlueprintCallable, Category = Start)
		void SetStartTile(ATile3D* startTile) { m_StartTile = startTile; }

	ATile3D* GetStartTile() { return m_StartTile; }

	// damage functions
	UFUNCTION(BlueprintCallable, Category = Damage)
		void DealDamage(float damage, AActor* damageInstigator) {
		m_Health -= damage;
		m_AttackingEnemy = damageInstigator;
	}

	float GetAttackRange() { return m_AttackRange; }
	float GetFollowRange() { return m_FollowRange; }

	// robustness functions
	UFUNCTION(BlueprintCallable, Category = Robust)
		bool ValidNPC() {
		if (m_SensorBrain == nullptr || m_MemoryBrain == nullptr || m_PlanningBrain == nullptr || m_EmotionBrain == nullptr) // invalid if any agent is null
		{
			return false;
		}
		return true;
	}

	// action functions

	void CallAction(Action action);

	void SetAction(Action action) { m_CurrentAction = action; }

	bool IsExecutingAction() {
		if (m_CurrentAction.actionType != Function::NullAction)
		{
			return true;
		}
		return false;
	}

	void SetRotation();

	// Death and respawn functions

	void Death();
	UFUNCTION(BlueprintCallable, Category = Respawn)
		void Respawn();

	bool ShouldRunTick() { return m_HasEscaped && m_HasDied; } // if either are false, then the tick will not run

	UFUNCTION(BlueprintCallable, Category = HasEscaped)
		bool GetHasEscaped() { return m_HasEscaped || !m_Threat; } // if not a threat npc this will always return true (so as to ignore their values)
	UFUNCTION(BlueprintCallable, Category = HasEscaped)
		bool GetHasDied() { return m_HasDied; }

	// memory functions

	void UpdateMemory(TArray<AActor*> actorsInView) { m_MemoryBrain->UpdateObjectsInMemory(actorsInView); } // call memory brain with new information

	TArray<TEnumAsByte<Quality>> GetQualitiesFromMemory() { return m_MemoryBrain->GetQualities(); }

	// null state

	State NULLState() {
		State nullState;
		nullState.tile = nullptr;
		nullState.actionState = ActionState::DoingNothing;
		return nullState;
	}

	// Emotion functions

	void SendMessageToEmotionBrain(TPair<Emotion, float> message);

	// Planning functions

	void CallSetGoal(State newGoal) { m_PlanningBrain->SetGoal(newGoal); }

	int32 GetIndex() { return m_Index; }

	float GetMaxHealth() { return m_MaxHealth; }

	float GetHealth() { return m_Health; }

	bool CallIsNPCInMemory() { return m_MemoryBrain->IsNPCInMemory(); }

	void ThisNPCEscaped() {
		m_PlanningBrain->GetInitialState().tile->SetType(TileType::None);
		m_HasEscaped = true; // has escaped
		Death(); // kill npc
	}

	float CallGetTimeBeforeLastScenario() { return m_PlanningBrain->GetTimeBeforeLastScenario(); }
	float CallGetLastTimeBeforeLastScenario() { return m_PlanningBrain->GetLastTimeBeforeLastScenario(); }
	float GetDamageDealt() { return m_DamageDealt; }
	float GetLastDamageDealt() { return m_LastDamageDealt; }

	// Goal creation getters

	AlertLevel GetAlertness() { return m_Alertness; }

	TArray<AActor*> GetCharactersInView() { return m_SensorBrain->GetAllObjectsWithinView(); }

	ATile3D* CallFindClosestTile(FVector2D tileLoc) { return m_PlanningBrain->FindClosestTile(tileLoc); }

	FVector2D CallGetLastSeenTileLocation() { return m_MemoryBrain->GetLastSeenTileLocation(); }

	TArray<ActorSnapShot> GetObjectsFromMemory() { return m_MemoryBrain->GetObjectsInMemory(); }

	bool CallIsActionQueueEmpty() { return m_PlanningBrain->IsActionQueueEmpty(); }

	bool GetEndOfScenario() { return m_EndOfScenario; }
	void SetEndOfScenario(bool newVal) { m_EndOfScenario = newVal; }

	// name generation & getter

	void GenerateName();

	float GetNameFitness(FString name);

	TArray<FString> GenerateStartConnectors();

	TArray<FString> GenerateMiddleConnectors();

	TArray<FString> GenerateEndConnectors();

	FString CreateConnectorName(int32 numberOfPairs);

	float GetPairingValidityFitness(char first, char second, char third, float& maxFitnessRef);

	bool IsVowel(char character);

	UFUNCTION(BlueprintCallable, Category = Name)
		FString GetName() { return m_Name; }
};


