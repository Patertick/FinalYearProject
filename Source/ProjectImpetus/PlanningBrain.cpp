// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanningBrain.h"
#include "Tile3D.h"
#include <Kismet/GameplayStatics.h>
#include "NPC.h"

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

	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);

	// SET POSSIBLE STATES

	State newState; // temp state
	for (AActor* tile : Tiles)
	{
		// add all possible tile and directive combinations to possible states
		newState.tile = Cast<ATile3D>(tile);
		newState.actionState = ActionState::DoingNothing;
		m_PossibleStates.Add(newState);
		newState.actionState = ActionState::Attacking;
		m_PossibleStates.Add(newState);
		newState.actionState = ActionState::Interacting;
		m_PossibleStates.Add(newState);

		m_MapData.Add(Cast<ATile3D>(tile));
	}

	// SET ACTIONS

	Action newAction;

	for (State state : m_PossibleStates)
	{
		for (ATile3D* connectedTile : state.tile->GetConnectedTiles())
		{

			// set possible move actions between connected tiles for each state
			newAction.startingState = state;
			// this state should already exist in possible states, but rather than waste time looking it up, simply set new state to equivalent state
			newState.actionState = state.actionState;
			newState.tile = connectedTile;
			newAction.endState = newState;
			newAction.actionType = Function::MoveFunction;

			if (newAction.startingState == newAction.endState)
			{
				// do nothing
			}
			else m_Actions.Add(newAction);



			// possible interact actions include states is interacting with each connected tile
			// end state is not where the NPC will be physically its just the state that the NPC is interacting with (as in whatever object is in that position)
			newAction.startingState = state;
			newState.actionState = ActionState::Interacting;
			newState.tile = connectedTile;
			newAction.endState = newState;
			newAction.actionType = Function::InteractFunction;
			if (newAction.startingState == newAction.endState)
			{
				// do nothing
			}
			else m_Actions.Add(newAction);
		}

		// possible attack actions include states where the tile contains an NPC & is within range
		for (ATile3D* tile : m_MapData)
		{
			// since the action is marked as an attack function, our NPC isn't going to the location that the state specifies, it's attacking that state
			// possible states are just any state any NPC can be in, thus using an attack function on a state is simply attacking whatever NPC is in that state
			newAction.startingState = state;
			// if tile is within range this is our target
			newState.actionState = ActionState::Attacking;
			newState.tile = tile;
			newAction.endState = newState;
			newAction.actionType = Function::AttackFunction;
			if (newAction.startingState == newAction.endState)
			{
				// do nothing
			}
			else m_Actions.Add(newAction);
		}
	}

	m_InitialState.actionState = ActionState::DoingNothing;
	m_InitialState.tile = nullptr;
	
}


// Called every frame
void UPlanningBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_InitialState.tile == nullptr)
	{
		FVector2D npcLocation = FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y };
		m_InitialState.tile = FindClosestTile(npcLocation);
		m_InitialState.tile->SetType(TileType::NPC);
	}

	// ...
	if (m_NPCRef->GetDirective() == Directive::MoveHere)
	{
		// empty actions queue
		m_ActionQueue.Empty();

		// find goal state

		State goalState;
		goalState.tile = nullptr;

		for (State state : m_PossibleStates)
		{
			if (Cast<ATile3D>(m_Focus) != nullptr)
			{
				if (state.actionState == ActionState::DoingNothing && state.tile == Cast<ATile3D>(m_Focus))
				{
					goalState = state;
					break;
				}
			}
		}

		if (goalState.tile == nullptr || goalState.tile == m_InitialState.tile)
		{
			return;
		}

		Path newPath = FindAStarPath(m_InitialState.tile, goalState.tile);


		

		if (newPath.totalCost < 0)
		{
			// try again
			return;
		}



		// find actions that follow path

		FVector2D startPointLocation = newPath.locations[0];
		newPath.locations.RemoveAt(0); // get rid of starting point so we dont waste an action going from start tile to start tile

		for (FVector2D location : newPath.locations)
		{
			// for each location
			for (Action action : m_Actions)
			{
				if (action.actionType == Function::MoveFunction)
				{
					// find the action that goes from start location to this new location
					FVector2D startingTileLocation = FVector2D{ action.startingState.tile->GetActorLocation().X, action.startingState.tile->GetActorLocation().Y };
					FVector2D endTileLocation = FVector2D{ action.endState.tile->GetActorLocation().X, action.endState.tile->GetActorLocation().Y };
					if (startingTileLocation.Equals(startPointLocation) && endTileLocation.Equals(location))
					{
						// add actions to queue
						if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
					}
				}
			}
			startPointLocation = location; // update start point
		}

		// set directive to do nothing

		m_NPCRef->SetDirective(Directive::DoNothing);
	}
	else if (m_NPCRef->GetDirective() == Directive::AttackThis)
	{
		// empty action queue

		m_ActionQueue.Empty();

		// Find goal state (attacking focus tile)

		State goalState;
		goalState.tile = nullptr;

		for (State state : m_PossibleStates)
		{
			if (Cast<ANPC>(m_Focus) != nullptr)
			{
				FVector2D otherNPCLocation = FVector2D{ m_Focus->GetActorLocation().X, m_Focus->GetActorLocation().Y };
				if (state.actionState == ActionState::Attacking && state.tile == FindClosestTile(otherNPCLocation))
				{
					goalState = state;
					break;
				}
			}
		}

		if (goalState.tile == nullptr || goalState.tile == m_InitialState.tile)
		{
			return;
		}

		// if goal state is in range from initial state, add attack to action queue

		if (IsWithinAttackRange(m_InitialState.tile->GetActorLocation(), goalState.tile->GetActorLocation()))
		{
			for (Action action : m_Actions)
			{
				if (action.startingState == m_InitialState && action.endState == goalState && action.actionType == Function::AttackFunction)
				{
					if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
				}
			}
		}
		else
		{
			// otherwise find A* path to a tile within range, add these moves to action queue then an attack

			// first find all tiles in range

			ATile3D* closestTile = nullptr;

			for (ATile3D* tile : m_MapData)
			{
				if (IsWithinAttackRange(tile->GetActorLocation(), goalState.tile->GetActorLocation()) && tile->GetType() == TileType::None)
				{

					// then find the closest tile to starting location
					if (closestTile == nullptr)
					{
						closestTile = tile;
					}
					else
					{
						FVector2D goalLocation = FVector2D{ goalState.tile->GetActorLocation().X, goalState.tile->GetActorLocation().Y };
						FVector2D closestTileLocation = FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y };
						FVector2D currentTileLocation = FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y };
						if (FVector2D::Distance(closestTileLocation, goalLocation) > FVector2D::Distance(currentTileLocation, goalLocation))
						{
							closestTile = tile;
						}
					}
				}
			}

			// pathfind to this tile

			Path newPath = FindAStarPath(m_InitialState.tile, closestTile);

			if (newPath.totalCost < 0)
			{
				// try again
				return;
			}

			// add subsequent move actions to action queue

			FVector2D startPointLocation = newPath.locations[0];
			newPath.locations.RemoveAt(0);

			for (FVector2D location : newPath.locations)
			{
				for (Action action : m_Actions)
				{
					if (action.actionType == Function::MoveFunction)
					{
						FVector2D startingTileLocation = FVector2D{ action.startingState.tile->GetActorLocation().X, action.startingState.tile->GetActorLocation().Y };
						FVector2D endTileLocation = FVector2D{ action.endState.tile->GetActorLocation().X, action.endState.tile->GetActorLocation().Y };
						if (startingTileLocation.Equals(startPointLocation) && endTileLocation.Equals(location))
						{
							if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
						}
					}
				}
				startPointLocation = location;
			}

			// add attack action to queue

			for (Action action : m_Actions)
			{
				if (action.startingState == m_InitialState && action.endState == goalState && action.actionType == Function::AttackFunction)
				{
					if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
				}
			}
		}

		// set directive to nothing

		m_NPCRef->SetDirective(Directive::DoNothing);
	}
	else if (m_NPCRef->GetDirective() == Directive::InteractThis)
	{
		// same logic as attack but with interaction

		m_ActionQueue.Empty();

		State goalState;
		goalState.tile = nullptr;

		for (State state : m_PossibleStates)
		{
			if (Cast<AInteractable>(m_Focus) != nullptr)
			{
				FVector2D interactableLocation = FVector2D{ m_Focus->GetActorLocation().X, m_Focus->GetActorLocation().Y };
				if (state.actionState == ActionState::Interacting && state.tile == FindClosestTile(interactableLocation))
				{
					goalState = state;
					break;
				}
			}
		}

		if (goalState.tile == nullptr || goalState.tile == m_InitialState.tile)
		{
			return;
		}

		if (IsConnectedTile(m_InitialState.tile, goalState.tile))
		{
			for (Action action : m_Actions)
			{
				if (action.startingState == m_InitialState && action.endState == goalState && action.actionType == Function::InteractFunction)
				{
					if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
				}
			}
		}
		else
		{

			ATile3D* closestTile = nullptr;

			for (ATile3D* tile : m_MapData)
			{
				// instead of finding tiles in range, find all tiles connected to goal tile
				if (IsConnectedTile(tile, goalState.tile) && tile->GetType() == TileType::None)
				{
					if (closestTile == nullptr)
					{
						closestTile = tile;
					}
					else
					{
						FVector2D goalLocation = FVector2D{ goalState.tile->GetActorLocation().X, goalState.tile->GetActorLocation().Y };
						FVector2D closestTileLocation = FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y };
						FVector2D currentTileLocation = FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y };
						if (FVector2D::Distance(closestTileLocation, goalLocation) > FVector2D::Distance(currentTileLocation, goalLocation))
						{
							closestTile = tile;
						}
					}
				}
			}

			Path newPath = FindAStarPath(m_InitialState.tile, closestTile);

			if (newPath.totalCost < 0)
			{
				// try again
				return;
			}

			FVector2D startPointLocation = newPath.locations[0];
			newPath.locations.RemoveAt(0);

			for (FVector2D location : newPath.locations)
			{
				for (Action action : m_Actions)
				{
					if (action.actionType == Function::MoveFunction)
					{
						FVector2D startingTileLocation = FVector2D{ action.startingState.tile->GetActorLocation().X, action.startingState.tile->GetActorLocation().Y };
						FVector2D endTileLocation = FVector2D{ action.endState.tile->GetActorLocation().X, action.endState.tile->GetActorLocation().Y };
						if (startingTileLocation.Equals(startPointLocation) && endTileLocation.Equals(location))
						{
							if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
						}
					}
				}
				startPointLocation = location;
			}

			for (Action action : m_Actions)
			{
				if (action.startingState == m_InitialState && action.endState == goalState && action.actionType == Function::InteractFunction)
				{
					if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
				}
			}
		}

		m_NPCRef->SetDirective(Directive::DoNothing);
	}
	else if (m_NPCRef->GetDirective() == Directive::FollowThis)
	{
		if (!m_NPCRef->IsExecutingAction())
		{
			// don't calculate path if action is being executed, waste of processing
			return;
		}

		// empty action queue

		m_ActionQueue.Empty();

		// get focus tile

		FVector2D focusLocation = FVector2D{ m_Focus->GetActorLocation().X, m_Focus->GetActorLocation().Y };
		ATile3D* focusTile = FindClosestTile(focusLocation);


		// find tiles within follow range of target

		ATile3D* closestTile = nullptr;

		for (ATile3D* tile : m_MapData)
		{
			// set equivalent goal state to closest tile
			if (IsWithinFollowRange(tile->GetActorLocation(), focusTile->GetActorLocation()) && tile->GetType() == TileType::None)
			{
				if (closestTile == nullptr)
				{
					closestTile = tile;
				}
				else
				{
					FVector2D goalLocation = FVector2D{ focusTile->GetActorLocation().X, focusTile->GetActorLocation().Y };
					FVector2D closestTileLocation = FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y };
					FVector2D currentTileLocation = FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y };
					if (FVector2D::Distance(closestTileLocation, goalLocation) > FVector2D::Distance(currentTileLocation, goalLocation))
					{
						closestTile = tile;
					}
				}
			}
		}

		if (closestTile == nullptr)
		{
			return;
		}

		// find A* path to goal

		Path newPath = FindAStarPath(m_InitialState.tile, closestTile);

		if (newPath.totalCost < 0)
		{
			// try again
			return;
		}

		// add subsequent actions

		FVector2D startPointLocation = newPath.locations[0];
		newPath.locations.RemoveAt(0);

		for (FVector2D location : newPath.locations)
		{
			for (Action action : m_Actions)
			{
				if (action.actionType == Function::MoveFunction)
				{
					FVector2D startingTileLocation = FVector2D{ action.startingState.tile->GetActorLocation().X, action.startingState.tile->GetActorLocation().Y };
					FVector2D endTileLocation = FVector2D{ action.endState.tile->GetActorLocation().X, action.endState.tile->GetActorLocation().Y };
					if (startingTileLocation.Equals(startPointLocation) && endTileLocation.Equals(location))
					{
						if (!m_ActionQueue.Contains(action)) m_ActionQueue.InsertItem(action);
					}
				}
			}
			startPointLocation = location;
		}

		// if action queue is non empty

		if (!m_ActionQueue.IsEmpty())
		{
			// run first action in queue
			Action action = m_ActionQueue.GetFirstItem();
			m_NPCRef->SetAction(action);
		}


		// otherwise do nothing
	}
	else if (m_NPCRef->GetDirective() == Directive::DoNothing)
	{
		if (!m_ActionQueue.IsEmpty())
		{
			// item exists in action queue and npc isn't currently doing something, run next action
			if (!m_NPCRef->IsExecutingAction())
			{
				Action action = m_ActionQueue.GetFirstItem();
				m_NPCRef->SetAction(action);
			}
		}
	}
	else
	{
		// error
	}


}

Path UPlanningBrain::FindAStarPath(ATile3D* startTile, ATile3D* endTile)
{
	if (startTile == nullptr || endTile == nullptr) {
		Path path;
		path.totalCost = -1;
		return path;
	}

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

bool UPlanningBrain::IsWithinAttackRange(FVector startingTileLocation, FVector targetTileLocation)
{
	FVector2D startTile2D = FVector2D{ startingTileLocation.X, startingTileLocation.Y };
	FVector2D targetTile2D = FVector2D{ targetTileLocation.X, targetTileLocation.Y };
	if (FVector2D::Distance(startTile2D, targetTile2D) <= m_NPCRef->GetAttackRange())
	{
		return true;
	}
	return false;
}

bool UPlanningBrain::IsWithinFollowRange(FVector startingTileLocation, FVector targetTileLocation)
{
	FVector2D startTile2D = FVector2D{ startingTileLocation.X, startingTileLocation.Y };
	FVector2D targetTile2D = FVector2D{ targetTileLocation.X, targetTileLocation.Y };
	if (FVector2D::Distance(startTile2D, targetTile2D) <= m_NPCRef->GetFollowRange())
	{
		return true;
	}
	return false;
}

bool UPlanningBrain::IsConnectedTile(ATile3D* startTile, ATile3D* endTile)
{
	if (endTile->GetConnectedTiles().Contains(startTile))
	{
		return true;
	}
	return false;
}

ATile3D* UPlanningBrain::FindClosestTile(FVector2D location)
{
	ATile3D* closestTile = nullptr;
	for (ATile3D* tile : m_MapData)
	{
		FVector2D tilePos = FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y };
		// find closest tile for initial state
		if (closestTile == nullptr) closestTile = tile;
		else if (FVector2D::Distance(tilePos, location) <
			FVector2D::Distance(FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y }, location))
		{
			// if current tile is closer to this NPC than last closest tile, set new closest tile to current tile
			closestTile = tile;
		}
	}
	return closestTile;
}
