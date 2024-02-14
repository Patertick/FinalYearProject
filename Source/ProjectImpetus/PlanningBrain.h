// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanningBrain.generated.h"

class ATile3D;

// for use in A* algorithm only start
struct Path {
	TArray<ATile3D*> tiles;
	float totalCost;
};

struct Node {
	float totalCostFromGoal;
	float actualCost;
	float heuristicCost;
	ATile3D* associatedTile;
	Node* parent;
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
	bool InList(const TArray<Node*>& list, ATile3D* tile);
	int FindRemoveIndex(const TArray<Node*>& list, Node* nodeToRemove);
		
};
