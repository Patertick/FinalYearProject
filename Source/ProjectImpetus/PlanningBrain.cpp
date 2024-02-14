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

	TArray<Node*> closedList; // checked tiles
	TArray<Node*> openList; // tiles to be checked

	Node* startNode = new Node();

	startNode->associatedTile = startTile;
	startNode->actualCost = 0;
	startNode->heuristicCost = FVector::Distance(startTile->GetActorLocation(), endTile->GetActorLocation());
	startNode->totalCostFromGoal = startNode->actualCost + startNode->heuristicCost;
	startNode->parent = nullptr;

	openList.Add(startNode);

	while (foundPath == false && openList.Num() != 0) // while there are tiles to be checked and thus path has not been found
	{
		// find node in open list with lowest total cost
		Node* closestNode = openList[0];
		for (Node* node : openList)
		{
			if (closestNode->totalCostFromGoal > node->totalCostFromGoal)
			{
				closestNode = node;
			}
		}

		// remove node from open list
		int index = FindRemoveIndex(openList, closestNode);
		if (index >= 0) openList.RemoveAt(index);

		TArray<ATile3D*> connectedTiles = closestNode->associatedTile->GetConnectedTiles();

		for (ATile3D* tile : connectedTiles)
		{
			// if goal has been found exit search
			if (tile == endTile)
			{
				Node* newNode = new Node();
				newNode->parent = closestNode;
				newNode->associatedTile = endTile;
				newNode->actualCost = closestNode->actualCost + tile->GetWeight();
				newNode->heuristicCost = FVector::Distance(tile->GetActorLocation(), endTile->GetActorLocation());
				newNode->totalCostFromGoal = newNode->actualCost + newNode->heuristicCost;
				closedList.Add(newNode);
				foundPath = true;
				break;
			}

			Node* newNode = new Node();
			newNode->parent = closestNode;
			newNode->associatedTile = tile;
			newNode->actualCost = closestNode->actualCost + tile->GetWeight();
			newNode->heuristicCost = FVector::Distance(tile->GetActorLocation(), endTile->GetActorLocation());
			newNode->totalCostFromGoal = newNode->actualCost + newNode->heuristicCost;

			if (!InList(openList, newNode->associatedTile) && !InList(closedList, newNode->associatedTile) && newNode->associatedTile->GetType() == TileType::None)
			{
				openList.Add(newNode);
			}
		}

		closedList.Add(closestNode);

		
	}

	if (InList(closedList, startTile) && InList(closedList, endTile)) // valid path
	{
		Path path;
		Node* currentNode = nullptr;
		for (Node* node : closedList)
		{
			if (node->associatedTile == endTile)
			{
				currentNode = node;
				break;
			}
		}
		if (currentNode != nullptr)
		{
			int count{ 0 };
			while (currentNode->associatedTile != startTile && count <= 100)
			{
				path.tiles.Add(currentNode->associatedTile);
				path.totalCost += currentNode->totalCostFromGoal;
				currentNode = currentNode->parent;
				count++;
			}
			for (ATile3D* tile : path.tiles)
			{
				if (Cast<ATile3D>(tile) != nullptr)
				{
					Cast<ATile3D>(tile)->m_IsSeen = true;
				}
			}

			//// collect pointers and delete them so as not to waste memory
			//for (int i = 0; i < closedList.Num(); i++)
			//{
			//	if (closedList[i] != nullptr)
			//	{
			//		delete closedList[i];
			//	}
			//}

			//for (int i = 0; i < openList.Num(); i++)
			//{
			//	if (openList[i] != nullptr)
			//	{
			//		delete openList[i];
			//	}
			//}

			//delete startNode;

			return path;
		}
	}

	/*for (int i = 0; i < closedList.Num(); i++)
	{
		if (closedList[i] != nullptr)
		{
			delete closedList[i];
		}
	}

	for (int i = 0; i < openList.Num(); i++)
	{
		if (openList[i] != nullptr)
		{
			delete openList[i];
		}
	}*/

	delete startNode;

	Path path;
	path.totalCost = -1; // invalid identifier
	return path;

}


bool UPlanningBrain::InList(const TArray<Node*>& list, ATile3D* tile)
{
	for (Node* node : list)
	{
		if (node->associatedTile == tile)
		{
			return true;
		}
	}
	return false;
}

int UPlanningBrain::FindRemoveIndex(const TArray<Node*>& list, Node* nodeToRemove)
{
	for (int i = 0; i < list.Num(); i++)
	{
		if (list[i]->associatedTile == nodeToRemove->associatedTile)
		{
			return i;
		}
	}
	return -1;
}
