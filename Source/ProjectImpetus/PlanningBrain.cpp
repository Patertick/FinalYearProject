// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanningBrain.h"
#include "Tile3D.h"

// Sets default values for this component's properties
UPlanningBrain::UPlanningBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlanningBrain::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlanningBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

Path UPlanningBrain::FindAStarPath(ATile3D* startTile, ATile3D* endTile)
{
	float heuristicCost{ 0 }; // going to use euclidean distance heuristic

	bool foundPath{ false };

	TArray<FNode> closedList; // checked tiles
	TArray<FNode> openList; // tiles to be checked
	

	heuristicCost = FVector::Distance(startTile->GetActorLocation(), endTile->GetActorLocation());
	FNode startNode = FNode(0, heuristicCost, startTile, nullptr);

	openList.Add(startNode);

	while (foundPath == false && openList.Num() != 0) // while there are tiles to be checked and thus path has not been found
	{
		// find node in open list with lowest total cost
		FNode closestNode = openList[0];
		for (FNode node : openList)
		{
			if (closestNode.totalCostFromGoal > node.totalCostFromGoal)
			{
				closestNode = node;
			}
		}

		// remove node from open list
		int index = FindRemoveIndex(openList, closestNode);
		if (index >= 0) openList.RemoveAt(index);

		TArray<ATile3D*> connectedTiles = closestNode.associatedTile->GetConnectedTiles();

		for (ATile3D* tile : connectedTiles)
		{
			// if goal has been found exit search
			if (tile == endTile)
			{
				heuristicCost = FVector::Distance(tile->GetActorLocation(), endTile->GetActorLocation());
				FNode newNode = FNode(closestNode.actualCost + tile->GetWeight(), heuristicCost, endTile, closestNode.associatedTile);
				closedList.Add(newNode);
				foundPath = true;
				break;
			}


			heuristicCost = FVector::Distance(tile->GetActorLocation(), endTile->GetActorLocation());
			FNode newNode = FNode(closestNode.actualCost + tile->GetWeight(), heuristicCost, tile, closestNode.associatedTile);

			if (!InList(openList, newNode.associatedTile) && !InList(closedList, newNode.associatedTile) && newNode.associatedTile->GetType() == TileType::None)
			{
				openList.Add(newNode);
			}
		}

		closedList.Add(closestNode);

		
	}

	if (InList(closedList, startTile) && InList(closedList, endTile)) // valid path
	{
		Path reversePath;
		FNode currentNode = FNode(0, 0, nullptr, nullptr);
		for (FNode node : closedList)
		{
			if (node.associatedTile == endTile)
			{
				currentNode = node;
				break;
			}
		}
		if (currentNode.associatedTile != nullptr)
		{
			while (currentNode.associatedTile != startTile)
			{
				reversePath.locations.Add(FVector2D{ currentNode.associatedTile->GetActorLocation().X, currentNode.associatedTile->GetActorLocation().Y });
				reversePath.totalCost += currentNode.totalCostFromGoal;
				for (FNode node : closedList)
				{
					if (node.associatedTile == currentNode.parentTile)
					{
						currentNode = node;
					}
				}
			}
			Path path;
			for (int i = reversePath.locations.Num() - 1; i >= 0; i--)
			{
				path.locations.Add(reversePath.locations[i]);
			}
			path.totalCost = reversePath.totalCost;

			if (path.totalCost < 0) path.totalCost = 1;

			return path;
		}
	}

	Path path;
	path.totalCost = -1; // invalid identifier
	return path;

}


bool UPlanningBrain::InList(const TArray<FNode>& list, ATile3D* tile)
{
	for (FNode node : list)
	{
		if (node.associatedTile == tile)
		{
			return true;
		}
	}
	return false;
}

int UPlanningBrain::FindRemoveIndex(const TArray<FNode>& list, FNode nodeToRemove)
{
	for (int i = 0; i < list.Num(); i++)
	{
		if (list[i].associatedTile == nodeToRemove.associatedTile)
		{
			return i;
		}
	}
	return -1;
}

//FVector2D UPlanningBrain::InterpolatePath(FVector2D startPoint, FVector2D endPoint, FVector2D currentLocation)
//{
//
//}
