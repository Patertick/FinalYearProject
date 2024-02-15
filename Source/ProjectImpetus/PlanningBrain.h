// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanningBrain.generated.h"

class ATile3D;

// for use in A* algorithm only
struct Path {
	TArray<FVector2D> locations;
	float totalCost;

	void operator=(Path other)
	{
		locations = other.locations;
		totalCost = other.totalCost;
	}
};

struct FNode {
	float totalCostFromGoal;
	float actualCost;
	float heuristicCost;
	ATile3D* associatedTile;
	ATile3D* parentTile;

	void operator=(FNode other)
	{
		totalCostFromGoal = other.totalCostFromGoal;
		actualCost = other.actualCost;
		heuristicCost = other.heuristicCost;
		associatedTile = other.associatedTile;
		parentTile = other.parentTile;
	}

	FNode(float actual, float heuristic, ATile3D* tileRef, ATile3D* prnt)
	{
		actualCost = actual;
		heuristicCost = heuristic;
		totalCostFromGoal = actual + heuristic;
		associatedTile = tileRef;
		parentTile = prnt;
	}
};

// State enum and struct

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

// Action enum and struct

UENUM()
enum Function
{
	MoveFunction,
	AttackFunction,
	InteractFunction,
	NullAction,
};

struct Action
{
	State startingState;
	State endState;
	Function actionType;
};

// Queue FIFO structure


struct ActionQueue {
	TArray<Action> items;

	void InsertItem(Action item)
	{
		items.Insert(item, 0);
	}
	Action GetFirstItem()
	{
		Action getItem = items[0];
		items.RemoveAt(0);
		return getItem;
	}
	void Empty()
	{
		items.Empty();
	}
	bool IsEmpty()
	{
		if (items.Num() <= 0)
		{
			return true;
		}
		return false;
	}
	bool Contains(Action item)
	{
		for (Action selfItem : items)
		{
			if (selfItem.actionType == item.actionType && selfItem.startingState == item.startingState && selfItem.endState == item.endState)
			{
				return true;
			}
		}
		return false;
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTIMPETUS_API UPlanningBrain : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlanningBrain();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	ANPC* m_NPCRef{ nullptr }; // reference to the object this brain works for

	AActor* m_Focus{ nullptr }; // NPCs current focus/objective

	// weights for use in altering A* (from djikstra's to best first search)
	float m_HeuristicWeight{ 0.5f };

	// action queue
	
	ActionQueue m_ActionQueue;

	// planning variables

	TArray<State> m_GoalStates; // possible states that apply to our goal

	TArray<State> m_PossibleStates; // all possible states for this NPC

	TArray<Action> m_Actions; // collection of possible actions

	State m_InitialState; // starting state; when action is called this is set to state after action

	// Map data

	TArray<ATile3D*> m_MapData; // data of every tile on the map

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }


	// Directive helper functions

	bool IsWithinAttackRange(FVector startingTileLocation, FVector targetTileLocation);
	bool IsWithinFollowRange(FVector startingTileLocation, FVector targetTileLocation);
	bool IsConnectedTile(ATile3D* startTile, ATile3D* endTile);
	ATile3D* FindClosestTile(FVector2D location);
	void SetFocus(AActor* focus) { m_Focus = focus; }
	AActor* GetFocus() { return m_Focus; }

	// A* functions

	// only start and end tile are needed, each tile stores connected tiles so that's our search space
	Path FindAStarPath(ATile3D* startTile, ATile3D* endTile);
	bool InList(const TArray<FNode>& list, ATile3D* tile);
	int FindRemoveIndex(const TArray<FNode>& list, FNode nodeToRemove);
	FVector2D InterpolatePath(FVector2D startPoint, FVector2D endPoint, FVector2D currentLocation);

	// get initial state and set initial state

	State GetInitialState() { return m_InitialState; }
	void SetInitialState(State newState) { m_InitialState = newState; }


		
};
