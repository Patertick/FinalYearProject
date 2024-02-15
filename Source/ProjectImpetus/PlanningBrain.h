// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanningBrain.generated.h"

class ATile3D;

// for use in A* algorithm only start
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
// for use in A* algorithm only end


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

	// weights for use in altering A* (from djikstra's to best first search)
	float m_HeuristicWeight{ 0.5f };

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	// only start and end tile are needed, each tile stores connected tiles so that's our search space
	Path FindAStarPath(ATile3D* startTile, ATile3D* endTile);
	bool InList(const TArray<FNode>& list, ATile3D* tile);
	int FindRemoveIndex(const TArray<FNode>& list, FNode nodeToRemove);
	FVector2D InterpolatePath(FVector2D startPoint, FVector2D endPoint, FVector2D currentLocation);
		
};
