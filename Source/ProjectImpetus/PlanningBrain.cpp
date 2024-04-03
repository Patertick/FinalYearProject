// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanningBrain.h"
#include "Tile3D.h"
#include <Kismet/GameplayStatics.h>
#include "NPC.h"
#include "Interactable.h"

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

	//Action newAction;

	//for (State state : m_PossibleStates)
	//{
	//	for (ConnectedTile connectedTile : state.tile->GetConnectedTiles())
	//	{

	//		// set possible move actions between connected tiles for each state
	//		newAction.startingState = state;
	//		// this state should already exist in possible states, but rather than waste time looking it up, simply set new state to equivalent state
	//		newState.actionState = state.actionState;
	//		newState.tile = connectedTile.ref;
	//		newAction.endState = newState;
	//		newAction.actionType = Function::MoveFunction;

	//		if (newAction.startingState == newAction.endState)
	//		{
	//			// do nothing
	//		}
	//		else m_Actions.Add(newAction);



	//		// possible interact actions include states is interacting with each connected tile
	//		// end state is not where the NPC will be physically its just the state that the NPC is interacting with (as in whatever object is in that position)
	//		newAction.startingState = state;
	//		newState.actionState = ActionState::Interacting;
	//		newState.tile = connectedTile.ref;
	//		newAction.endState = newState;
	//		newAction.actionType = Function::InteractFunction;
	//		if (newAction.startingState == newAction.endState)
	//		{
	//			// do nothing
	//		}
	//		else m_Actions.Add(newAction);
	//	}

	//	// possible attack actions include states where the tile contains an NPC & is within range
	//	for (ATile3D* tile : m_MapData)
	//	{
	//		// since the action is marked as an attack function, our NPC isn't going to the location that the state specifies, it's attacking that state
	//		// possible states are just any state any NPC can be in, thus using an attack function on a state is simply attacking whatever NPC is in that state
	//		newAction.startingState = state;
	//		// if tile is within range this is our target
	//		newState.actionState = ActionState::Attacking;
	//		newState.tile = tile;
	//		newAction.endState = newState;
	//		newAction.actionType = Function::AttackFunction;
	//		if (newAction.startingState == newAction.endState)
	//		{
	//			// do nothing
	//		}
	//		else m_Actions.Add(newAction);
	//	}
	//}

	//m_InitialState.actionState = ActionState::DoingNothing;
	
}

void UPlanningBrain::AddMovePathToActionQueue(Path movePath)
{
	for (int i = 0; i < movePath.locations.Num() - 1; i++)
	{
		// for each location
		// create the action that goes from start location to this new location
		Action newAction;
		newAction.actionType = Function::MoveFunction;
		newAction.startingState.actionState = ActionState::DoingNothing;
		newAction.startingState.tile = FindClosestTile(movePath.locations[i]);
		newAction.endState.actionState = ActionState::DoingNothing;
		newAction.endState.tile = FindClosestTile(movePath.locations[i + 1]);
		// add action to queue
		m_ActionQueue.InsertItem(newAction);

	}
}


// Called every frame
void UPlanningBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	return;
	if (m_NPCRef->GetHasDied()) return;

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
				//AppendMapStateManager(m_InitialState.tile, NPCAction::NPCDoingNothingSelf);
				Action action = m_ActionQueue.GetFirstItem();
				m_NPCRef->SetAction(action);
			}
		}
		else
		{
			// is this NPC an enemy?
			if (m_NPCRef->m_Threat)
			{

				TArray<AActor*> NPCs;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);

				TArray<ANPC*> friendlyNPC;
				for (int i = 0; i < NPCs.Num(); i++)
				{
					if (Cast<ANPC>(NPCs[i]) != nullptr)
					{
						if (!Cast<ANPC>(NPCs[i])->m_Threat)
						{
							friendlyNPC.Add(Cast<ANPC>(NPCs[i]));
						}
					}
				}

				// is NPC in view?
				if (friendlyNPC.Num() > 0)
				{

					// search adjacent tiles
					TArray<ConnectedTile> adjacentTiles = m_InitialState.tile->GetConnectedTiles();
					ConnectedTile newTile;
					for (ConnectedTile tile : adjacentTiles)
					{
						// has adjacent tile got an NPC on it?
						if (tile.ref->GetType() == TileType::NPC)
						{
							// run attack function on tile then exit
							Action newAction;
							newAction.actionType = Function::AttackFunction;
							newAction.direction = GetDirection(m_InitialState.tile, tile.ref);
							newAction.startingState = m_InitialState;
							newAction.endState.tile = tile.ref;
							newAction.endState.actionState = ActionState::Attacking;
							m_ActionQueue.InsertItem(newAction);
							return;
						}
					}

					// find closest NPC in view

					TArray<ANPC*> actorsInView = friendlyNPC;

					AActor* closestActor = nullptr;

					for (ANPC* actor : friendlyNPC)
					{
						if (!actor->GetHasDied())
						{
							if (closestActor == nullptr)
							{
								closestActor = actor;
							}
							else
							{
								float oldDistance = FVector2D::Distance(FVector2D{ closestActor->GetActorLocation().X, closestActor->GetActorLocation().Y }, FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
								float newDistance = FVector2D::Distance(FVector2D{ actor->GetActorLocation().X, actor->GetActorLocation().Y }, FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
								if (newDistance < oldDistance)
								{
									closestActor = actor;
								}
							}
						}

					}

					if (closestActor == nullptr) return;

					if (FindClosestTile(FVector2D{ closestActor->GetActorLocation().X, closestActor->GetActorLocation().Y }) == FindClosestTile(FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y }))
					{
						// run attack function on tile then exit
						UGameplayStatics::ApplyDamage(closestActor, 100.0f, nullptr, m_NPCRef, NULL); // instantly kill NPC
						return;
					}

					// find closest adjacent tile to closest NPC

					ATile3D* closestTile = nullptr;

					for (ConnectedTile tile : adjacentTiles)
					{
						if (closestTile == nullptr)
						{
							closestTile = tile.ref;
						}
						else
						{
							float oldDistance = FVector2D::Distance(FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y }, FVector2D{ closestActor->GetActorLocation().X, closestActor->GetActorLocation().Y });
							float newDistance = FVector2D::Distance(FVector2D{ tile.ref->GetActorLocation().X, tile.ref->GetActorLocation().Y }, FVector2D{ closestActor->GetActorLocation().X, closestActor->GetActorLocation().Y });
							if (newDistance < oldDistance)
							{
								closestTile = tile.ref;
							}
						}

					}

					if (closestTile == nullptr) return;

					// is the next step tile a floor tile?
					if (closestTile->GetType() != TileType::Wall)
					{
						// run move action towards this tile
						Action newAction;
						newAction.actionType = Function::MoveFunction;
						newAction.direction = GetDirection(m_InitialState.tile, closestTile);
						newAction.startingState = m_InitialState;
						newAction.endState.tile = closestTile;
						newAction.endState.actionState = ActionState::MovingToLocation;
						m_ActionQueue.InsertItem(newAction);
						return;
					}
					else
					{
						// run ability action on this tile (turns wall tiles to floor tiles)
						Action newAction;
						newAction.actionType = Function::AbilityFunction;
						newAction.direction = GetDirection(m_InitialState.tile, closestTile);
						newAction.startingState = m_InitialState;
						newAction.endState.tile = closestTile;
						newAction.endState.actionState = ActionState::UsingAbility;
						m_ActionQueue.InsertItem(newAction);
						return;
					}



				}
				else
				{
					// random move
					TArray<ConnectedTile> adjacentTiles = m_InitialState.tile->GetConnectedTiles();
					TArray<ATile3D*> traversableConnectedTiles;
					for (ConnectedTile tile : adjacentTiles)
					{
						// is adjacent tile a dirt tile?
						if (tile.ref->GetType() == TileType::None || tile.ref->GetType() == TileType::Escape)
						{
							traversableConnectedTiles.Add(tile.ref);
						}
					}

					ATile3D* randomTile;

					randomTile = traversableConnectedTiles[FMath::RandRange(0, traversableConnectedTiles.Num() - 1)];


					// is the next step tile a floor tile?
					if (randomTile->GetType() == TileType::None)
					{
						// run move action towards this tile
						Action newAction;
						newAction.actionType = Function::MoveFunction;
						newAction.direction = GetDirection(m_InitialState.tile, randomTile);
						newAction.startingState = m_InitialState;
						newAction.endState.tile = randomTile;
						newAction.endState.actionState = ActionState::MovingToLocation;
						m_ActionQueue.InsertItem(newAction);
						return;
					}
					else if (randomTile->GetType() == TileType::Wall)
					{
						// run ability action on this tile (turns wall tiles to floor tiles)
						Action newAction;
						newAction.actionType = Function::AbilityFunction;
						newAction.direction = GetDirection(m_InitialState.tile, randomTile);
						newAction.startingState = m_InitialState;
						newAction.endState.tile = randomTile;
						newAction.endState.actionState = ActionState::UsingAbility;
						m_ActionQueue.InsertItem(newAction);
						return;
					}
				}
			}
			else
			{
				if (m_CurrentSetOfActions.Num() <= 0)
				{
					// generate random set of actions
					TArray<Action> generatedSetOfActions = GenerateArrayOfActions();
					// add to action queue
					m_CurrentSetOfActions = generatedSetOfActions;
					m_ActionQueue.InsertItems(m_CurrentSetOfActions);
					m_TimeBeforeLastScenario = 0.0f;
				}

				// on end of scenario
				if (m_NPCRef->GetEndOfScenario())
				{
					m_NPCRef->SetEndOfScenario(false);
					
					// evaluate past actions success
					if (m_MutatedSetOfActions.Num() <= 0)
					{
						// create mutated set of actions and add to action queue
						m_MutatedSetOfActions = MutateActions(m_CurrentSetOfActions);
						m_ActionQueue.InsertItems(m_MutatedSetOfActions);
						return;
					}

					// was the new set of actions more successful?
					if (m_TimeBeforeLastScenario > m_LastTimeBeforeLastScenario)
					{
						// override the old set of actions
						m_CurrentSetOfActions = m_MutatedSetOfActions;
					}
					else
					{
						if (EvaluateActions(m_MutatedSetOfActions) > EvaluateActions(m_CurrentSetOfActions))
						{
							m_CurrentSetOfActions = m_MutatedSetOfActions;
						}
					}

					m_LastTimeBeforeLastScenario = m_TimeBeforeLastScenario;
					m_TimeBeforeLastScenario = 0.0f;

					// using current set of actions stored mutate it and add to action queue
					m_MutatedSetOfActions = MutateActions(m_CurrentSetOfActions);
					m_ActionQueue.InsertItems(m_MutatedSetOfActions);
					return;
				}

				m_TimeBeforeLastScenario += DeltaTime;

			}


		}
	}
	else
	{
		// error
	}


}

TArray<Action> UPlanningBrain::MutateActions(const TArray<Action>& actions)
{
	TArray<Action> newActionArray = actions;

	// delete 10 to 30 actions
	int32 randomDeleteInt = FMath::RandRange(10, 30);
	for (int i = 0; i < randomDeleteInt; i++)
	{
		int32 randomIndex = FMath::RandRange(0, newActionArray.Num() - 1);
		newActionArray.RemoveAt(randomIndex);
	}
	// create deleted items to deleted items + 10 new actions
	int32 randomAddInt = FMath::RandRange(randomDeleteInt, randomDeleteInt + 10);
	for (int i = 0; i < randomAddInt; i++)
	{

		// convert to random action
		GenerateRandomAction();

		FString action = GenerateRandomAction();
		State currentState;
		if (i == 0)
		{
			currentState.actionState = ActionState::DoingNothing;
			currentState.tile = m_NPCRef->GetStartTile();
		}
		else
		{
			currentState.actionState = newActionArray[i - 1].endState.actionState;
			currentState.tile = newActionArray[i - 1].endState.tile;
		}

		if (action.GetCharArray()[0] == 'A')
		{
			ConnectedTile attackTile{ nullptr };
			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					attackTile.ref = tile.ref;
					attackTile.xDir = tile.xDir;
					attackTile.yDir = tile.yDir;
					break;
				}
			}

			// send attack action to queue
			Action newAction;
			newAction.actionType = Function::AttackFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::Attacking;
			newAction.endState.tile = attackTile.ref;
			newAction.direction = FVector2D{ attackTile.xDir, attackTile.yDir };
			newActionArray.Add(newAction);

		}
		else if (action.GetCharArray()[0] == 'M')
		{
			ConnectedTile moveTile{ nullptr };
			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					moveTile.ref = tile.ref;
					moveTile.xDir = tile.xDir;
					moveTile.yDir = tile.yDir;
					break;
				}
			}

			// send attack action to queue
			Action newAction;
			newAction.actionType = Function::MoveFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::MovingToLocation;
			newAction.endState.tile = moveTile.ref;
			newAction.direction = FVector2D{ moveTile.xDir, moveTile.yDir };
			newActionArray.Add(newAction);


		}
		else if (action.GetCharArray()[0] == 'I')
		{
			ConnectedTile interactTile{ nullptr };
			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					interactTile.ref = tile.ref;
					interactTile.xDir = tile.xDir;
					interactTile.yDir = tile.yDir;
					break;
				}
			}

			// send attack action to queue
			Action newAction;
			newAction.actionType = Function::InteractFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::Interacting;
			newAction.endState.tile = interactTile.ref;
			newAction.direction = FVector2D{ interactTile.xDir, interactTile.yDir };
			newActionArray.Add(newAction);

		}
		else if (action.GetCharArray()[0] == 'S')
		{
			ConnectedTile abilityTile{ nullptr };
			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					abilityTile.ref = tile.ref;
					abilityTile.xDir = tile.xDir;
					abilityTile.yDir = tile.yDir;
					break;
				}

			}
			Action newAction;
			newAction.actionType = Function::AbilityFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::UsingAbility;
			newAction.endState.tile = abilityTile.ref;
			newAction.direction = FVector2D{ abilityTile.xDir, abilityTile.yDir };
			newActionArray.Add(newAction);

		}


	
	}
	
	
	return newActionArray;
}

float UPlanningBrain::EvaluateActions(const TArray<Action>& actions)
{
	float totalWeight{ 0.0f };
	for (Quality quality : m_NPCRef->GetQualitiesFromMemory())
	{
		if (quality == Quality::Fearless)
		{
			// find damage taken
			float damageTaken = m_NPCRef->GetMaxHealth() - m_NPCRef->GetHealth();
			// divide by max health (so the value is between 1 and 0; between  max damage taken and no damage taken)
			damageTaken = damageTaken / m_NPCRef->GetMaxHealth();
			totalWeight += damageTaken;
		}
		else if (quality == Quality::Coward)
		{
			// find damage taken
			float damageTaken = m_NPCRef->GetMaxHealth() - m_NPCRef->GetHealth();
			// divide by max health (so the value is between 1 and 0; between  max damage taken and no damage taken)
			damageTaken = damageTaken / m_NPCRef->GetMaxHealth();
			totalWeight += 1.0f - damageTaken; // do 1 minus damage taken so that less damage taken results in higher weight
		}
		else if (quality == Quality::MoralCompass)
		{
			TArray<AActor*> NPCs;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);

			float totalTimeAlive{ 0.0f };
			float lastTotalTimeAlive{ 0.0f };
			int32 numberOfFriendlyNPCS{ 0 };
			for (int i = 0; i < NPCs.Num(); i++)
			{
				if (Cast<ANPC>(NPCs[i]) != nullptr)
				{
					if (!Cast<ANPC>(NPCs[i])->m_Threat)
					{
						totalTimeAlive += Cast<ANPC>(NPCs[i])->CallGetTimeBeforeLastScenario();
						lastTotalTimeAlive += Cast<ANPC>(NPCs[i])->CallGetLastTimeBeforeLastScenario();
						numberOfFriendlyNPCS++;
					}
				}
			}

			if (numberOfFriendlyNPCS <= 0)
			{
				// don't add any weight
			}
			else
			{
				// find time spent alive for all other non-enemy NPCs
				// if the average time spent alive is higher then this should result in higher weight (thus add 1.0)
				totalTimeAlive /= numberOfFriendlyNPCS;
				lastTotalTimeAlive /= numberOfFriendlyNPCS;

				if (totalTimeAlive > lastTotalTimeAlive)
				{
					totalWeight += 1.0f;
				}

			}

			
		}
		else if (quality == Quality::Evil)
		{
			TArray<AActor*> NPCs;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);

			float totalTimeAlive{ 0.0f };
			float lastTotalTimeAlive{ 0.0f };
			int32 numberOfFriendlyNPCS{ 0 };
			for (int i = 0; i < NPCs.Num(); i++)
			{
				if (Cast<ANPC>(NPCs[i]) != nullptr)
				{
					if (!Cast<ANPC>(NPCs[i])->m_Threat)
					{
						totalTimeAlive += Cast<ANPC>(NPCs[i])->CallGetTimeBeforeLastScenario();
						lastTotalTimeAlive += Cast<ANPC>(NPCs[i])->CallGetLastTimeBeforeLastScenario();
						numberOfFriendlyNPCS++;
					}
				}
			}

			if (numberOfFriendlyNPCS <= 0)
			{
				// don't add any weight
			}
			else
			{
				// find time spent alive for all other non-enemy NPCs
				// if the average time spent alive is lower then this should result in higher weight (thus add 1.0)
				totalTimeAlive /= numberOfFriendlyNPCS;
				lastTotalTimeAlive /= numberOfFriendlyNPCS;

				if (totalTimeAlive < lastTotalTimeAlive)
				{
					totalWeight += 1.0f;
				}

			}
		}
		else if (quality == Quality::Violent)
		{
			// find damage dealt
			// higher damage since last scenario has higher weight (thus add 1.0)
			if (m_NPCRef->GetDamageDealt() > m_NPCRef->GetLastDamageDealt())
			{
				totalWeight += 1.0f;
			}
		}
		else if (quality == Quality::Pacifist)
		{
			// find damage dealt
			// lower damage since last scenario has higher weight (thus add 1.0)
			if (m_NPCRef->GetDamageDealt() < m_NPCRef->GetLastDamageDealt())
			{
				totalWeight += 1.0f;
			}
		}
		else if (quality == Quality::Efficient)
		{
			// go through actions and find redundant actions, divide the number of redundant actions by number of actions
			// return 1 - the calculated value
			totalWeight += 1.0f - FindNumberOfRedundantActions(m_CurrentSetOfActions);
			
		}
		else if (quality == Quality::Lazy)
		{
			// simply find number of actions and convert to a weight
			// lower number of actions have higher weights (thus add 1.0)
			if (m_MutatedSetOfActions.Num() < m_CurrentSetOfActions.Num())
			{
				totalWeight += 1.0f;
			}
		}
		else if (quality == Quality::Active)
		{
			// simply find number of actions and convert to a weight
			// lower number of actions have higher weights (thus add 1.0)
			if (m_MutatedSetOfActions.Num() > m_CurrentSetOfActions.Num())
			{
				totalWeight += 1.0f;
			}
		}
	}

	return totalWeight / m_NPCRef->GetQualitiesFromMemory().Num(); // return the average weight
}

int32 UPlanningBrain::FindNumberOfRedundantActions(const TArray<Action>& actions)
{
	// go through array, save actions to an array, if the array of actions overlaps with the next action then its redundant, increment a value and return once whole array has been checked
	int32 numberOfRedundantActions{ 0 };
	TArray<Action> uniqueActions;
	for (Action action : actions)
	{
		if (uniqueActions.Contains(action))
		{
			numberOfRedundantActions++;
		}
		else
		{
			uniqueActions.Add(action);
		}
	}
	return numberOfRedundantActions / actions.Num();
}

TArray<Action> UPlanningBrain::GenerateArrayOfActions()
{
	int32 randLength = FMath::RandRange(50, 100);
	TArray<Action> newActionArray;
	State currentState;
	currentState.actionState = ActionState::DoingNothing;
	currentState.tile = m_NPCRef->GetStartTile();
	for (int i = 0; i < randLength; i++)
	{
		FString action = GenerateRandomAction();
		if (action.GetCharArray()[0] == 'A')
		{
			ConnectedTile attackTile{ nullptr };

			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					attackTile.ref = tile.ref;
					attackTile.xDir = tile.xDir;
					attackTile.yDir = tile.yDir;
					break;
				}
			}

			// send attack action to queue
			Action newAction;
			newAction.actionType = Function::AttackFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::Attacking;
			newAction.endState.tile = attackTile.ref;
			newAction.direction = FVector2D{ attackTile.xDir, attackTile.yDir };
			newActionArray.Add(newAction);

		}
		else if (action.GetCharArray()[0] == 'M')
		{
			ConnectedTile moveTile{ nullptr };
			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					moveTile.ref = tile.ref;
					moveTile.xDir = tile.xDir;
					moveTile.yDir = tile.yDir;
					break;
				}
			}

			// send attack action to queue
			Action newAction;
			newAction.actionType = Function::MoveFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::MovingToLocation;
			newAction.endState.tile = moveTile.ref;
			newAction.direction = FVector2D{ moveTile.xDir, moveTile.yDir };
			newActionArray.Add(newAction);
			currentState.tile = newAction.endState.tile; // update current state


		}
		else if (action.GetCharArray()[0] == 'I')
		{
			ConnectedTile interactTile{ nullptr };
			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					interactTile.ref = tile.ref;
					interactTile.xDir = tile.xDir;
					interactTile.yDir = tile.yDir;
					break;
				}
			}

			// send attack action to queue
			Action newAction;
			newAction.actionType = Function::InteractFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::Interacting;
			newAction.endState.tile = interactTile.ref;
			newAction.direction = FVector2D{ interactTile.xDir, interactTile.yDir };
			newActionArray.Add(newAction);

		}
		else if (action.GetCharArray()[0] == 'S')
		{
			ConnectedTile abilityTile{ nullptr };
			TArray<ConnectedTile> connectedTiles = currentState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == action.GetCharArray()[1])
				{
					// this is the direction we want
					abilityTile.ref = tile.ref;
					abilityTile.xDir = tile.xDir;
					abilityTile.yDir = tile.yDir;
					break;
				}
			}

			Action newAction;
			newAction.actionType = Function::AbilityFunction;
			newAction.startingState.actionState = currentState.actionState;
			newAction.startingState.tile = currentState.tile;
			newAction.endState.actionState = ActionState::UsingAbility;
			newAction.endState.tile = abilityTile.ref;
			newAction.direction = FVector2D{ abilityTile.xDir, abilityTile.yDir };
			newActionArray.Add(newAction);

		}
	}
	return newActionArray;
}

FVector2D UPlanningBrain::GetDirection(ATile3D* startTile, ATile3D* endTile)
{
	FVector2D direction;
	// get X difference (get from A to B is B - A)
	float xDiff = endTile->GetActorLocation().X - startTile->GetActorLocation().X;
	// find direction (-1 is left, 1 is right, 0 is no direction)
	if (xDiff < -0.5f)
	{
		direction.X = 1;
	}
	else if (xDiff > 0.5f)
	{
		direction.X = -1;
	}
	else
	{
		direction.X = 0;
	}
	
	// do the same with y
	float yDiff = endTile->GetActorLocation().Y - startTile->GetActorLocation().Y;
	if (yDiff < -0.5f)
	{
		direction.Y = 1;
	}
	else if (yDiff > 0.5f)
	{
		direction.Y = -1;
	}
	else
	{
		direction.Y = 0;
	}
	return direction;
}

// Q learning functions


void UPlanningBrain::CreateNextRandomAction()
{
	if (m_MapStateManager->GetMapState().npcStates.Num() <= 0) return;
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
		if (attackTile.ref == nullptr || !TileHasNPC(attackTile.ref) || 
			FindClosestNPC(FVector2D{ attackTile.ref->GetActorLocation().X, attackTile.ref->GetActorLocation().Y })->m_Threat == m_NPCRef->m_Threat) return;
		else
		{
			// send attack action to queue
			Action newAction;
			QNode newNode;
			newAction.actionType = Function::AttackFunction;
			newAction.startingState.actionState = m_InitialState.actionState;
			newAction.startingState.tile = m_InitialState.tile;
			newAction.endState.actionState = ActionState::Attacking;
			newAction.endState.tile = attackTile.ref;
			newAction.direction = FVector2D{ attackTile.xDir, attackTile.yDir };
			if (actionString.GetCharArray()[1] == 'u')
			{
				newNode.action = NPCAction::NPCAttackUp;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'r')
			{
				newNode.action = NPCAction::NPCAttackRight;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'l')
			{
				newNode.action = NPCAction::NPCAttackLeft;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'd')
			{
				newNode.action = NPCAction::NPCAttackDown;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else
			{
				newNode.action = NPCAction::NPCAttackSelf;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			m_PastActionsQNodes.Add(newNode);
			AppendMapStateManager(m_InitialState.tile, newNode.action);
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
		if (moveTile.ref == nullptr || moveTile.ref->GetType() != TileType::None) return;
		else
		{
			// send attack action to queue
			Action newAction;
			QNode newNode;
			newAction.actionType = Function::MoveFunction;
			newAction.startingState.actionState = m_InitialState.actionState;
			newAction.startingState.tile = m_InitialState.tile;
			newAction.endState.actionState = ActionState::MovingToLocation;
			newAction.endState.tile = moveTile.ref;
			newAction.direction = FVector2D{ moveTile.xDir, moveTile.yDir };
			if (actionString.GetCharArray()[1] == 'u')
			{
				newNode.action = NPCAction::NPCMoveUp;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'r')
			{
				newNode.action = NPCAction::NPCMoveRight;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'l')
			{
				newNode.action = NPCAction::NPCMoveLeft;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'd')
			{
				newNode.action = NPCAction::NPCMoveDown;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			m_PastActionsQNodes.Add(newNode);
			AppendMapStateManager(m_InitialState.tile, newNode.action);
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
		if (interactTile.ref == nullptr || !TileHasInteractable(interactTile.ref)) return;
		else
		{
			// send attack action to queue
			Action newAction;
			QNode newNode;
			newAction.actionType = Function::InteractFunction;
			newAction.startingState.actionState = m_InitialState.actionState;
			newAction.startingState.tile = m_InitialState.tile;
			newAction.endState.actionState = ActionState::Interacting;
			newAction.endState.tile = interactTile.ref;
			newAction.direction = FVector2D{ interactTile.xDir, interactTile.yDir };

			if (actionString.GetCharArray()[1] == 'u')
			{
				newNode.action = NPCAction::NPCInteractUp;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'r')
			{
				newNode.action = NPCAction::NPCInteractRight;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'l')
			{
				newNode.action = NPCAction::NPCInteractLeft;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'd')
			{
				newNode.action = NPCAction::NPCInteractDown;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else
			{
				newNode.action = NPCAction::NPCInteractSelf;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			m_PastActionsQNodes.Add(newNode);
			AppendMapStateManager(m_InitialState.tile, newNode.action);
			// add action to queue
			m_ActionQueue.InsertItem(newAction);
		}
	} 
	else if (actionString.GetCharArray()[0] == 'S')
	{
		ConnectedTile abilityTile{ nullptr };
		if (actionString.GetCharArray()[1] == 'n')
		{
			// no direction , do not find connected tiles
			abilityTile.ref = m_InitialState.tile;
			abilityTile.xDir = 0;
			abilityTile.yDir = 0;
		}
		{
			TArray<ConnectedTile> connectedTiles = m_InitialState.tile->GetConnectedTiles();
			for (ConnectedTile tile : connectedTiles)
			{
				if (tile.direction.GetCharArray()[0] == actionString.GetCharArray()[1])
				{
					// this is the direction we want
					abilityTile.ref = tile.ref;
					abilityTile.xDir = tile.xDir;
					abilityTile.yDir = tile.yDir;
					break;
				}
			}
		}
		if (abilityTile.ref == nullptr) return;
		else
		{
			// send attack action to queue
			Action newAction;
			QNode newNode;
			newAction.actionType = Function::AbilityFunction;
			newAction.startingState.actionState = m_InitialState.actionState;
			newAction.startingState.tile = m_InitialState.tile;
			newAction.endState.actionState = ActionState::UsingAbility;
			newAction.endState.tile = abilityTile.ref;
			newAction.direction = FVector2D{ abilityTile.xDir, abilityTile.yDir };
			if (actionString.GetCharArray()[1] == 'u')
			{
				newNode.action = NPCAction::NPCAbilityUp;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'r')
			{
				newNode.action = NPCAction::NPCAbilityRight;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'l')
			{
				newNode.action = NPCAction::NPCAbilityLeft;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else if (actionString.GetCharArray()[1] == 'd')
			{
				newNode.action = NPCAction::NPCAbilityDown;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			else
			{
				newNode.action = NPCAction::NPCAbilitySelf;
				newNode.worldState = m_MapStateManager->GetMapState();
			}
			m_PastActionsQNodes.Add(newNode);
			AppendMapStateManager(m_InitialState.tile, newNode.action);
			// add action to queue
			m_ActionQueue.InsertItem(newAction);
		}
		// nothing happens for else no need to load action
	}
} 

float UPlanningBrain::EvaluateAction()
{
	if (m_NPCRef->m_Threat)
	{
		// check health of all other npcs 
		// how much damage was dealt?


		float damageDealtTotal{ 0.0f };
		int32 count{ 0 };

		for (const NPCState& oldStates : m_PastActionsQNodes[0].worldState.npcStates)
		{
			if (oldStates.NPCIndex != m_NPCRef->GetIndex())
			{
				for (const NPCState& newStates : m_MapStateManager->GetMapState().npcStates)
				{
					if (newStates.NPCIndex != m_NPCRef->GetIndex())
					{
						float damageDealt;
						damageDealt = abs(newStates.npcHealth - oldStates.npcHealth);
						damageDealt /= m_NPCRef->GetMaxHealth();
						damageDealtTotal += damageDealt;
						count++;
						// divide by max health to enforce between 0.0f and 1.0f
					}
				}
			}
		}

		return damageDealtTotal / count; // higher differences in health mean higher values for these actions
	}
	else
	{
		// check health from first state to current state
		// how much damage was taken?

		float damageTaken{ 0.0f };

		for (const NPCState& oldStates : m_PastActionsQNodes[0].worldState.npcStates)
		{
			if (oldStates.NPCIndex == m_NPCRef->GetIndex())
			{
				for (const NPCState& newStates : m_MapStateManager->GetMapState().npcStates)
				{
					if (newStates.NPCIndex == m_NPCRef->GetIndex())
					{
						damageTaken =abs(newStates.npcHealth - oldStates.npcHealth);
						damageTaken /= m_NPCRef->GetMaxHealth();
						// divide by max health to enforce between 0.0f and 1.0f
					}
				}
			}
		}

		return  1.0f - damageTaken; // return 1 minus the difference between health values, if theres a high difference value will be lower, if theres a low difference then value with be higher
	}
	return 0.0f;
}

bool UPlanningBrain::WasLastActionSignificant()
{
	// significant actions include: NPC health changing value, NPC death, NPC using ability

	if (m_PastActionsQNodes.Num() <= 0) return false;

	if (m_PastActionsQNodes[m_PastActionsQNodes.Num() - 1].action == NPCAction::NPCAbilityDown || m_PastActionsQNodes[m_PastActionsQNodes.Num() - 1].action == NPCAction::NPCAbilityLeft
		|| m_PastActionsQNodes[m_PastActionsQNodes.Num() - 1].action == NPCAction::NPCAbilityUp || m_PastActionsQNodes[m_PastActionsQNodes.Num() - 1].action == NPCAction::NPCAbilityRight
		|| m_PastActionsQNodes[m_PastActionsQNodes.Num() - 1].action == NPCAction::NPCAbilitySelf)
	{
		return true;
	}


	for (const NPCState& oldStates : m_PastActionsQNodes[m_PastActionsQNodes.Num() - 1].worldState.npcStates)
	{
		for (const NPCState& newStates : m_MapStateManager->GetMapState().npcStates)
		{
			float healthDifference = oldStates.npcHealth - newStates.npcHealth;

			if (healthDifference <= 0.01f && healthDifference >= -0.01f)
			{
				return true;
			}

			if (newStates.npcHealth <= 0.0f)
			{
				return true;
			}
		}
	}

	return false;
}

void UPlanningBrain::AddNPCToMapStateManager(int32 index, float health, FVector position)
{
	m_MapStateManager->AddToMapState(index, m_InitialState.tile, health, FVector2D{ position.X, position.Y });
}

void UPlanningBrain::AppendMapStateManager(ATile3D* currentTile, NPCAction currentAction)
{
	m_MapStateManager->AppendNPCState(m_NPCRef->GetIndex(), currentTile, currentAction, m_NPCRef->GetHealth(), FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
}

void UPlanningBrain::CreateNextAction()
{
	if (m_QValues.Num() <= 0) return;
	QNode highestValueNode = QNode();
	for (QNode node : m_QValues)
	{
		if (node.worldState == m_MapStateManager->GetMapState())
		{
			// find highest Q node value for this state
			if (highestValueNode.value < node.value)
			{
				highestValueNode = node;
			}
		}
	}

	if (highestValueNode == QNode()) return; // if there is no Q value for this state, return

	Action newAction;
	TArray<ConnectedTile> connectedTiles = m_InitialState.tile->GetConnectedTiles();


	QNode newNode;
	newNode.action = highestValueNode.action;
	newNode.worldState = m_MapStateManager->GetMapState();
	AppendMapStateManager(m_InitialState.tile, newNode.action);
	m_PastActionsQNodes.Add(newNode);
	
	

	switch (highestValueNode.action)
	{
	case NPCAbilityUp:
		
		newAction.actionType = Function::AbilityFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::UsingAbility;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'u')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAbilityDown:
		newAction.actionType = Function::AbilityFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::UsingAbility;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'd')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAbilityLeft:
		newAction.actionType = Function::AbilityFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::UsingAbility;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'l')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAbilityRight:
		newAction.actionType = Function::AbilityFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::UsingAbility;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'r')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAbilitySelf:
		newAction.actionType = Function::AbilityFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::UsingAbility;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'n')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAttackUp:
		newAction.actionType = Function::AttackFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Attacking;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'u')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasNPC(tile.ref) ||
					FindClosestNPC(FVector2D{ tile.ref->GetActorLocation().X, tile.ref->GetActorLocation().Y })->m_Threat == m_NPCRef->m_Threat) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAttackDown:
		newAction.actionType = Function::AttackFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Attacking;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'd')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasNPC(tile.ref) ||
					FindClosestNPC(FVector2D{ tile.ref->GetActorLocation().X, tile.ref->GetActorLocation().Y })->m_Threat == m_NPCRef->m_Threat) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAttackLeft:
		newAction.actionType = Function::AttackFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Attacking;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'l')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasNPC(tile.ref) ||
					FindClosestNPC(FVector2D{ tile.ref->GetActorLocation().X, tile.ref->GetActorLocation().Y })->m_Threat == m_NPCRef->m_Threat) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAttackRight:
		newAction.actionType = Function::AttackFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Attacking;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'r')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasNPC(tile.ref) ||
					FindClosestNPC(FVector2D{ tile.ref->GetActorLocation().X, tile.ref->GetActorLocation().Y })->m_Threat == m_NPCRef->m_Threat) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCAttackSelf:
		newAction.actionType = Function::AttackFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Attacking;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'n')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasNPC(tile.ref) ||
					FindClosestNPC(FVector2D{ tile.ref->GetActorLocation().X, tile.ref->GetActorLocation().Y })->m_Threat == m_NPCRef->m_Threat) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCInteractUp:
		newAction.actionType = Function::InteractFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Interacting;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'u')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasInteractable(tile.ref)) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCInteractDown:
		newAction.actionType = Function::InteractFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Interacting;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'd')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasInteractable(tile.ref)) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCInteractLeft:
		newAction.actionType = Function::InteractFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Interacting;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'l')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasInteractable(tile.ref)) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCInteractRight:
		newAction.actionType = Function::InteractFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Interacting;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'r')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasInteractable(tile.ref)) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCInteractSelf:
		newAction.actionType = Function::InteractFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::Interacting;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'n')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || !TileHasInteractable(tile.ref)) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCMoveUp:
		newAction.actionType = Function::MoveFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::MovingToLocation;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'u')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || tile.ref->GetType() != TileType::None) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCMoveDown:
		newAction.actionType = Function::MoveFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::MovingToLocation;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'd')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || tile.ref->GetType() != TileType::None) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCMoveLeft:
		newAction.actionType = Function::MoveFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::MovingToLocation;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'l')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || tile.ref->GetType() != TileType::None) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	case NPCMoveRight:
		newAction.actionType = Function::MoveFunction;
		newAction.startingState = m_InitialState;
		newAction.endState.actionState = ActionState::MovingToLocation;
		for (ConnectedTile tile : connectedTiles)
		{
			if (tile.direction.GetCharArray()[0] == 'r')
			{
				// this is the direction we want
				newAction.direction = FVector2D{ tile.xDir, tile.yDir };
				newAction.endState.tile = tile.ref;
				if (tile.ref == nullptr || tile.ref->GetType() != TileType::None) return;
				m_ActionQueue.InsertItem(newAction);
				return;
			}
		}
		break;
	default:
		break;
	}
}


void UPlanningBrain::CreateQValues(WorldState state)
{
	// find current map state
	// find current action

	if (m_PastActionsQNodes.Num() <= 0)
	{
		return;
	}
	

	// go through Q values, does this already exist?
	for (QNode node : m_QValues)
	{
		if (state == node.worldState)
		{
			// yes, return
			return;
		}
	}

	// create a node for every action

	QNode newQNode;
	newQNode.action = NPCAction::NPCAbilityDown;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAbilityLeft;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAbilityRight;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAbilityUp;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAbilitySelf;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAttackDown;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAttackLeft;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAttackRight;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAttackUp;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCAttackSelf;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCInteractDown;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCInteractLeft;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCInteractRight;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCInteractUp;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCInteractSelf;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCMoveDown;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCMoveLeft;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCMoveRight;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
	newQNode.action = NPCAction::NPCMoveUp;
	newQNode.value = 0.5f;
	newQNode.worldState = state;
	m_QValues.Add(newQNode);
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
	for (const FNode& node : list)
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
		outputString = outputString + "S"; // special ability
	}

	// generate random direction (assert that directions are lowercase)
	int32 randomDirection = FMath::RandRange(0, 3);

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
	else
	{
		outputString = outputString + "d"; // down
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
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);
	ATile3D* closestTile = nullptr;
	for (AActor* tile : Tiles)
	{
		FVector2D tilePos = FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y };
		// find closest tile for initial state
		if (closestTile == nullptr) closestTile = Cast<ATile3D>(tile);
		else if (FVector2D::Distance(tilePos, location) <
			FVector2D::Distance(FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y }, location))
		{
			// if current tile is closer to this NPC than last closest tile, set new closest tile to current tile
			closestTile = Cast<ATile3D>(tile);
		}
	}
	return closestTile;
}

ANPC* UPlanningBrain::FindClosestNPC(FVector2D location)
{
	TArray<AActor*> NPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);
	ANPC* closestNPC = nullptr;
	for (AActor* npc : NPCs)
	{
		FVector2D objectPos = FVector2D{ npc->GetActorLocation().X, npc->GetActorLocation().Y };
		// find closest tile for initial state
		if (closestNPC == nullptr) closestNPC = Cast<ANPC>(npc);
		else if (FVector2D::Distance(objectPos, location) <
			FVector2D::Distance(FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y }, location))
		{
			// if current tile is closer to this NPC than last closest tile, set new closest tile to current tile
			closestNPC = Cast<ANPC>(npc);
		}
	}
	return closestNPC;
}

AInteractable* UPlanningBrain::FindClosestInteractable(FVector2D location)
{
	TArray<AActor*> Interactables;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractable::StaticClass(), Interactables);
	AInteractable* closestInteractable = nullptr;
	for (AActor* interactable : Interactables)
	{
		FVector2D objectPos = FVector2D{ interactable->GetActorLocation().X, interactable->GetActorLocation().Y };
		// find closest tile for initial state
		if (closestInteractable == nullptr) closestInteractable = Cast<AInteractable>(interactable);
		else if (FVector2D::Distance(objectPos, location) <
			FVector2D::Distance(FVector2D{ closestInteractable->GetActorLocation().X, closestInteractable->GetActorLocation().Y }, location))
		{
			// if current tile is closer to this NPC than last closest tile, set new closest tile to current tile
			closestInteractable = Cast<AInteractable>(interactable);
		}
	}
	return closestInteractable;
}

bool UPlanningBrain::TileHasNPC(ATile3D* tile)
{
	ANPC* closestNPC = FindClosestNPC(FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y });
	if (closestNPC == nullptr) return false;
	FVector2D npcPos = FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y };

	if (FVector2D::Distance(FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y }, npcPos) < KTILESIZE) return true;
	return false;
}

bool UPlanningBrain::TileHasInteractable(ATile3D* tile)
{
	AInteractable* closestInteractable = FindClosestInteractable(FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y });
	if (closestInteractable == nullptr) return false;
	FVector2D interactablePos = FVector2D{ closestInteractable->GetActorLocation().X, closestInteractable->GetActorLocation().Y };

	if (FVector2D::Distance(FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y }, interactablePos) < KTILESIZE) return true;
	return false;
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