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
		for (ConnectedTile connectedTile : state.tile->GetConnectedTiles())
		{

			// set possible move actions between connected tiles for each state
			newAction.startingState = state;
			// this state should already exist in possible states, but rather than waste time looking it up, simply set new state to equivalent state
			newState.actionState = state.actionState;
			newState.tile = connectedTile.ref;
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
			newState.tile = connectedTile.ref;
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

	//m_InitialState.actionState = ActionState::DoingNothing;

	m_AutonomousGoal.actionState = ActionState::DoingNothing;
	
}


// Called every frame
void UPlanningBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
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

		if (goalState.tile == nullptr || goalState.tile == m_InitialState.tile || m_InitialState.tile == nullptr)
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

		for (int i = 0; i < newPath.locations.Num() - 1; i++)
		{
			// for each location
			// create the action that goes from start location to this new location
			Action newAction;
			newAction.actionType = Function::MoveFunction;
			newAction.startingState.actionState = ActionState::DoingNothing;
			newAction.startingState.tile = FindClosestTile(newPath.locations[i]);
			newAction.endState.actionState = ActionState::DoingNothing;
			newAction.endState.tile = FindClosestTile(newPath.locations[i + 1]);
			// add action to queue
			m_ActionQueue.InsertItem(newAction);
			
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
			m_ActionQueue.InsertItems(CreateAttackActions(6, goalState, m_InitialState.tile));
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
						FVector2D goalLocation = FVector2D{ m_InitialState.tile->GetActorLocation().X, m_InitialState.tile->GetActorLocation().Y };
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

			Action newAction;
			newAction.endState.tile = nullptr;
			for (int i = 0; i < newPath.locations.Num() - 1; i++)
			{
				// for each location
				// create the action that goes from start location to this new location
				newAction.actionType = Function::MoveFunction;
				newAction.startingState.actionState = ActionState::DoingNothing;
				newAction.startingState.tile = FindClosestTile(newPath.locations[i]);
				newAction.endState.actionState = ActionState::DoingNothing;
				newAction.endState.tile = FindClosestTile(newPath.locations[i + 1]);
				// add action to queue
				m_ActionQueue.InsertItem(newAction);

			}

			// add attack action to queue
			if (newAction.endState.tile == nullptr) return;
			m_ActionQueue.InsertItems(CreateAttackActions(6, goalState, newAction.endState.tile));
				
			
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
			Action newAction;
			newAction.actionType = Function::InteractFunction;
			newAction.startingState.actionState = ActionState::Interacting;
			newAction.startingState.tile = m_InitialState.tile;
			newAction.endState.actionState = ActionState::Interacting;
			newAction.endState.tile = goalState.tile;
			m_ActionQueue.InsertItem(newAction);
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
						FVector2D goalLocation = FVector2D{ m_InitialState.tile->GetActorLocation().X, m_InitialState.tile->GetActorLocation().Y };
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

			Action newAction;

			for (int i = 0; i < newPath.locations.Num() - 1; i++)
			{
				// for each location
				// create the action that goes from start location to this new location
				newAction.actionType = Function::MoveFunction;
				newAction.startingState.actionState = ActionState::DoingNothing;
				newAction.startingState.tile = FindClosestTile(newPath.locations[i]);
				newAction.endState.actionState = ActionState::DoingNothing;
				newAction.endState.tile = FindClosestTile(newPath.locations[i + 1]);
				// add action to queue
				m_ActionQueue.InsertItem(newAction);

			}
			newAction.actionType = Function::InteractFunction;
			newAction.startingState.actionState = ActionState::Interacting;
			newAction.startingState.tile = newAction.endState.tile; // use last end tile as new starting point for action
			newAction.endState.actionState = ActionState::Interacting;
			newAction.endState.tile = goalState.tile;
			m_ActionQueue.InsertItem(newAction);
		}

		m_NPCRef->SetDirective(Directive::DoNothing);
	}
	else if (m_NPCRef->GetDirective() == Directive::FollowThis)
	{
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
					FVector2D goalLocation = FVector2D{ m_InitialState.tile->GetActorLocation().X, m_InitialState.tile->GetActorLocation().Y };
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

		for (int i = 0; i < newPath.locations.Num() - 1; i++)
		{
			// for each location
			// create the action that goes from start location to this new location
			Action newAction;
			newAction.actionType = Function::MoveFunction;
			newAction.startingState.actionState = ActionState::DoingNothing;
			newAction.startingState.tile = FindClosestTile(newPath.locations[i]);
			newAction.endState.actionState = ActionState::DoingNothing;
			newAction.endState.tile = FindClosestTile(newPath.locations[i + 1]);
			// add action to queue
			m_ActionQueue.InsertItem(newAction);

		}

		// if action queue is non empty

		if (!m_ActionQueue.IsEmpty())
		{
			// run first action in queue
			if (!m_ActionQueue.IsEmpty())
			{
				if (!m_NPCRef->IsExecutingAction())
				{
					Action action = m_ActionQueue.GetFirstItem();
					m_NPCRef->SetAction(action);
				}
			}
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
		else
		{
			// generate random actions

			int32 numOfActions = FMath::RandRange(0, 20);

			for (int i = 0; i < numOfActions; i++)
			{
				// generate random action (move, attack, interact and then random direction)
				FString actionString = GenerateRandomAction();

				if (actionString.GetCharArray()[0] == 'A')
				{
					ConnectedTile attackTile{ nullptr };
					if (actionString.GetCharArray()[1] == 'n')
					{
						// no direction , do not find connected tiles
						attackTile.ref = m_InitialState.tile;
						attackTile.xDir = 0;
						attackTile.yDir = 0;
					}
					{
						TArray<ConnectedTile> connectedTiles = m_InitialState.tile->GetConnectedTiles();
						for (ConnectedTile tile : connectedTiles)
						{
							if (tile.direction.GetCharArray()[0] == actionString.GetCharArray()[1])
							{
								// this is the direction we want
								attackTile.ref = tile.ref;
								attackTile.xDir = tile.xDir;
								attackTile.yDir = tile.yDir;
								break;
							}
						}
					}
					if (attackTile.ref == nullptr) return;
					else
					{
						// send attack action to queue
						Action newAction;
						newAction.actionType = Function::AttackFunction;
						newAction.startingState.actionState = ActionState::Attacking;
						newAction.startingState.tile = m_InitialState.tile;
						newAction.endState.actionState = ActionState::Attacking;
						newAction.endState.tile = attackTile.ref;
						newAction.direction = FVector2D{ attackTile.xDir, attackTile.yDir };
						// add action to queue
						m_ActionQueue.InsertItem(newAction);
					}
				}
				else if (actionString.GetCharArray()[0] == 'M')
				{
					ConnectedTile moveTile{ nullptr };
					if (actionString.GetCharArray()[1] == 'n') return; // should not be able to move in no direction
					{
						TArray<ConnectedTile> connectedTiles = m_InitialState.tile->GetConnectedTiles();
						for (ConnectedTile tile : connectedTiles)
						{
							if (tile.direction.GetCharArray()[0] == actionString.GetCharArray()[1])
							{
								// this is the direction we want
								moveTile.ref = tile.ref;
								moveTile.xDir = tile.xDir;
								moveTile.yDir = tile.yDir;
								break;
							}
						}
					}
					if (moveTile.ref == nullptr) return;
					else
					{
						// send attack action to queue
						Action newAction;
						newAction.actionType = Function::MoveFunction;
						newAction.startingState.actionState = ActionState::DoingNothing;
						newAction.startingState.tile = m_InitialState.tile;
						newAction.endState.actionState = ActionState::DoingNothing;
						newAction.endState.tile = moveTile.ref;
						newAction.direction = FVector2D{ moveTile.xDir, moveTile.yDir };
						// add action to queue
						m_ActionQueue.InsertItem(newAction);
					}
				}
				else if (actionString.GetCharArray()[0] == 'I')
				{
					ConnectedTile interactTile{ nullptr };
					if (actionString.GetCharArray()[1] == 'n')
					{
						// no direction , do not find connected tiles
						interactTile.ref = m_InitialState.tile;
						interactTile.xDir = 0;
						interactTile.yDir = 0;
					}
					{
						TArray<ConnectedTile> connectedTiles = m_InitialState.tile->GetConnectedTiles();
						for (ConnectedTile tile : connectedTiles)
						{
							if (tile.direction.GetCharArray()[0] == actionString.GetCharArray()[1])
							{
								// this is the direction we want
								interactTile.ref = tile.ref;
								interactTile.xDir = tile.xDir;
								interactTile.yDir = tile.yDir;
								break;
							}
						}
					}
					if (interactTile.ref == nullptr) return;
					else
					{
						// send attack action to queue
						Action newAction;
						newAction.actionType = Function::InteractFunction;
						newAction.startingState.actionState = ActionState::Interacting;
						newAction.startingState.tile = m_InitialState.tile;
						newAction.endState.actionState = ActionState::Interacting;
						newAction.endState.tile = interactTile.ref;
						newAction.direction = FVector2D{ interactTile.xDir, interactTile.yDir };
						// add action to queue
						m_ActionQueue.InsertItem(newAction);
					}
				} // nothing happens for else no need to load action
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

		TArray <ConnectedTile> connectedTiles = closestNode.associatedTile->GetConnectedTiles();

		for (ConnectedTile tile : connectedTiles)
		{
			// if goal has been found exit search
			if (tile.ref == endTile)
			{
				heuristicCost = FVector::Distance(tile.ref->GetActorLocation(), endTile->GetActorLocation());
				FNode newNode = FNode(closestNode.actualCost + tile.ref->GetWeight(), heuristicCost, endTile, closestNode.associatedTile);
				closedList.Add(newNode);
				foundPath = true;
				break;
			}


			heuristicCost = FVector::Distance(tile.ref->GetActorLocation(), endTile->GetActorLocation());
			FNode newNode = FNode(closestNode.actualCost + tile.ref->GetWeight(), heuristicCost, tile.ref, closestNode.associatedTile);

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

FString UPlanningBrain::GenerateRandomAction()
{
	FString outputString = "";

	// generate action (move, attack or interact) (assert that actions are uppercase)

	int32 randomAction = FMath::RandRange(0, 3);

	if (randomAction == 0)
	{
		outputString = outputString + "A"; // attack
	}
	else if (randomAction == 1)
	{
		outputString = outputString + "M"; // move
	}
	else if (randomAction == 2)
	{
		outputString = outputString + "I"; // interact
	}
	else
	{
		outputString = outputString + "N"; // Nothing
	}

	// generate random direction (assert that directions are lowercase)

	int32 randomDirection = FMath::RandRange(0, 4);

	if (randomDirection == 0)
	{
		outputString = outputString + "u"; // up
	}
	else if (randomDirection == 1)
	{
		outputString = outputString + "l"; // left
	}
	else if (randomDirection == 2)
	{
		outputString = outputString + "r"; // right
	}
	else if (randomDirection == 3)
	{
		outputString = outputString + "d"; // down
	}
	else
	{
		outputString = outputString + "n"; // no direction
	}

	return outputString;
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
	TArray<ATile3D*> tiles;
	for (ConnectedTile tile : endTile->GetConnectedTiles())
	{
		tiles.Add(tile.ref);
	}
	if (tiles.Contains(startTile))
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

TArray<Action> UPlanningBrain::CreateAttackActions(int32 numberOfAttacks, State goal, ATile3D* startTile)
{
	TArray<Action> attacks;
	for (int i = 0; i < numberOfAttacks; i++)
	{
		Action newAction;
		newAction.actionType = Function::AttackFunction;
		newAction.startingState.actionState = ActionState::Attacking;
		newAction.startingState.tile = startTile;
		newAction.endState.actionState = ActionState::Attacking;
		newAction.endState.tile = goal.tile;
		attacks.Add(newAction);
	}
	return attacks;
}