// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
ANPC::ANPC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();


	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);

	// SET POSSIBLE STATES

	State newState; // temp state
	AActor* closestTile = nullptr;
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

		FVector2D actorPos = FVector2D{ GetActorLocation().X, GetActorLocation().Y};
		FVector2D tilePos = FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y };
		// find closest tile for initial state
		if (closestTile == nullptr) closestTile = tile;
		else if (FVector2D::Distance(tilePos, actorPos) < 
			FVector2D::Distance(FVector2D{closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y}, actorPos))
		{
			// if current tile is closer to this NPC than last closest tile, set new closest tile to current tile
			closestTile = tile;
		}
	}

	// SET INITIAL STATE
	
	m_InitialState.tile = Cast<ATile3D>(closestTile);
	m_InitialState.actionState = ActionState::DoingNothing; // start off doing nothing
	m_InitialState.tile->SetType(TileType::NPC);

	// set current action to null
	m_CurrentAction.actionType = Function::NullAction;
	m_CurrentAction.startingState = m_InitialState;
	m_CurrentAction.endState = m_InitialState;

	// set current path to invalid

	m_CurrentPath.totalCost = -1;

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

		
	
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ValidNPC()) return;

	if (m_Health <= 0.0f)
	{
		m_InitialState.tile->SetType(TileType::None);
		Destroy(); // death
		return;
	}

	for (ATile3D* tile : m_MapData)
	{
		tile->m_IsSeen = false;
	}
	for (FVector2D location : m_CurrentPath.locations)
	{
		FindClosestTile(location)->m_IsSeen = true;
	}


	m_SensorBrain->SetFieldOfView(m_FieldOfView);
	m_SensorBrain->SetMaxViewDistance(m_MaxViewDistance);

	m_Controllable = !m_Threat;

	// when the NPC has no action, try to find a new one given the NPC has a focus (or goal)
	if (m_CurrentAction.actionType == Function::NullAction && m_Focus != nullptr)
	{
		m_GoalStates.Empty(); // empty goal states
		switch (m_Directive)
		{
		case Directive::AttackThis:

			// make possible goal states for attacking (goal states being attacking & within range of focus)
			for (State state : m_PossibleStates)
			{
				if (state.actionState == ActionState::Attacking)
				{
					// attacking state
					if (FVector2D::Distance(FVector2D{ m_Focus->GetActorLocation().X, m_Focus->GetActorLocation().Y },
						FVector2D{ state.tile->GetActorLocation().X,state.tile->GetActorLocation().Y }) <= m_AttackRange)
					{
						m_GoalStates.Add(state); // within range and attacking, this state is a possible goal state
					}
				}
			}

			for (Action action : m_Actions)
			{
				for (State goal : m_GoalStates)
				{

					if (action.startingState == m_InitialState && action.endState == goal && action.actionType == Function::AttackFunction)
					{
						FVector2D focusLoc = FVector2D{ m_Focus->GetActorLocation().X, m_Focus->GetActorLocation().Y };
						FVector2D tilePos = FVector2D{ action.endState.tile->GetActorLocation().X, action.endState.tile->GetActorLocation().Y };
						if (action.endState.tile->GetType() == TileType::NPC &&
							FVector2D::Distance(focusLoc, tilePos) <= KTILEMAXDISTANCE)
						{// don't want to target a different NPC's tile, so inflict a restriction on max distance from the focus and tile
							m_CurrentAction = action;
							break;
						}
					}
				}
			}
			break;
			// following isn't really its own action, more of a set of move actions with restrictions, thus following directive will be used primarily
			// by planning brain
		case Directive::FollowThis:
			//// make possible goal states for following (goal states being within range of focus)
			//for (State state : m_PossibleStates)
			//{
			//	if (FVector::Distance(GetActorLocation(), state.tile->GetActorLocation()) <= m_FollowRange)
			//	{
			//		m_GoalStates.Add(state); // within range
			//	}
			//	
			//}
			break;
		case Directive::InteractThis:
			// make possible goal states for interacting (goal states being within range of focus & interacting)
			// make possible goal states for attacking (goal states being attacking & within range of focus)
			for (State state : m_PossibleStates)
			{
				if (state.actionState == ActionState::Interacting)
				{
					// Interacting state
					m_GoalStates.Add(state); // within range and interacting, this state is a possible goal state
				}
			}

			for (Action action : m_Actions)
			{
				for (State goal : m_GoalStates)
				{
					if (action.startingState == m_InitialState && action.endState == goal && action.actionType == Function::InteractFunction)
					{
						FVector2D focusLoc = FVector2D{ m_Focus->GetActorLocation().X, m_Focus->GetActorLocation().Y };
						FVector2D tilePos = FVector2D{ action.endState.tile->GetActorLocation().X, action.endState.tile->GetActorLocation().Y };
						if (action.endState.tile->GetType() == TileType::Object)
						{
							m_CurrentAction = action;
							break;
						}
					}
				}
			}

			break;
		case Directive::MoveHere:
			// make possible goal states for moving (goal states being the position of focus)
			//for (State state : m_PossibleStates)
			//{
			//	if (m_Focus->GetActorLocation().Equals(state.tile->GetActorLocation())) // find tiles that match location of focus
			//	{
			//		m_GoalStates.Add(state); // within range, this state is a possible goal state
			//	}
			//}

			//for (Action action : m_Actions)
			//{
			//	for (State goal : m_GoalStates)
			//	{

			//		if (action.startingState == m_InitialState && action.endState == goal && action.actionType == Function::MoveFunction)
			//		{
			//			m_CurrentAction = action;
			//			break;
			//		}
			//	}
			//}
			if (Cast<ATile3D>(m_Focus) != nullptr)
			{
				if (m_InitialState.tile != Cast<ATile3D>(m_Focus))
				{
					m_CurrentPath = m_PlanningBrain->FindAStarPath(m_InitialState.tile, Cast<ATile3D>(m_Focus));
					m_PointOnPath = 0;
					m_CurrentAction.actionType = Function::MoveFunction;
				}
				else
				{
					m_CurrentPath.totalCost = -1; // path has reached end
				}
			}




			break;
		case Directive::DoNothing:
		default:
			// no possible goal states
			break;
		}

		
		
	}
	CallAction(m_CurrentAction);

}

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ANPC::MovePath(Path path)
{
	// interpolation function
	// TO DO
	// movement
	// TEMP
	if (m_PointOnPath >= m_CurrentPath.locations.Num() - 1) 
	{
		// reached end of path, make current path null
		m_CurrentPath.totalCost = -1;
		m_CurrentAction.actionType = Function::NullAction;
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("MovePathSuccess"));

	FVector2D currentPos = FVector2D{ GetActorLocation().X, GetActorLocation().Y };

	FVector2D nextPoint = path.locations[m_PointOnPath + 1];

	FVector2D endPos = nextPoint;
	// use 2D vectors for movement as Z is constrained
	FVector2D moveVector = (endPos - currentPos).GetSafeNormal();
	moveVector *= m_WalkSpeed;
	FVector newLocation = GetActorLocation() + FVector{ moveVector.X, moveVector.Y, 0.0f };
	SetActorLocation(newLocation);
	currentPos = FVector2D{ newLocation.X, newLocation.Y };

	if (currentPos.Equals(endPos, m_WalkSpeed))
	{
		// successful move
		m_InitialState.tile = FindClosestTile(nextPoint);
		FindClosestTile(path.locations[m_PointOnPath])->SetType(TileType::None);
		FindClosestTile(nextPoint)->SetType(TileType::NPC);
		m_PointOnPath++;
		SetActorLocation(FVector{ m_InitialState.tile->GetActorLocation().X, m_InitialState.tile->GetActorLocation().Y, m_HalfHeight });
	}

}

void ANPC::Move(State startState, State endState)
{
	// move between tiles
	if (!startState.tile->GetConnectedTiles().Contains(endState.tile) || endState.tile->GetType() != TileType::None)
	{
		m_InitialState.tile = startState.tile;
		m_InitialState.actionState = endState.actionState;
		m_InitialState.tile->SetType(TileType::NPC);
		m_CurrentAction.actionType = Function::NullAction;
		m_Directive = Directive::DoNothing;
		return;
	}

	FVector2D currentPos = FVector2D{ GetActorLocation().X, GetActorLocation().Y };
	FVector2D endPos = FVector2D{ endState.tile->GetActorLocation().X, endState.tile->GetActorLocation().Y };
	// use 2D vectors for movement as Z is constrained
	FVector2D moveVector = (endPos - currentPos).GetSafeNormal();
	moveVector *= m_WalkSpeed;
	FVector newLocation = GetActorLocation() + FVector{ moveVector.X, moveVector.Y, 0.0f };
	SetActorLocation(newLocation);
	currentPos = FVector2D{ newLocation.X, newLocation.Y };

	if (currentPos.Equals(endPos, m_WalkSpeed))
	{
		// successful move
		m_InitialState.tile = endState.tile;

		SetActorLocation(FVector{ endState.tile->GetActorLocation().X, endState.tile->GetActorLocation().Y, m_HalfHeight });

		m_InitialState.actionState = endState.actionState;
		m_InitialState.tile->SetType(TileType::NPC);
		startState.tile->SetType(TileType::None);
		m_CurrentAction.actionType = Function::NullAction;
		m_Directive = Directive::DoNothing;
	}
}

void ANPC::Attack(State startState, State endState)
{
	// while target on endstate is alive
	if (endState.tile->GetType() == TileType::NPC && m_Focus != nullptr)
	{
		if (FVector2D::Distance(FVector2D{ startState.tile->GetActorLocation().X, startState.tile->GetActorLocation().Y }, 
			FVector2D{ endState.tile->GetActorLocation().X, endState.tile->GetActorLocation().Y }) <= m_AttackRange)
		{
			// if endstate is within range of startstate and the endstate tile type is defined as having an NPC on it, then apply damage to that npc
			UGameplayStatics::ApplyDamage(m_Focus, m_Damage, nullptr, this, NULL);
		}
		else {
			// exit action
			m_InitialState.tile = startState.tile; // attacking function, do not change tile position 
			m_InitialState.actionState = endState.actionState; // attack has finished, reset directive
			m_CurrentAction.actionType = Function::NullAction;
			m_Directive = Directive::DoNothing;			
		}
		return;
	}
	// exit action
	m_InitialState.tile = startState.tile;
	m_InitialState.actionState = endState.actionState;
	m_CurrentAction.actionType = Function::NullAction;
	m_Directive = Directive::DoNothing;
	
}

void ANPC::Interact(State startState, State endState)
{
	if (endState.tile->GetType() == TileType::Object && m_Focus != nullptr)
	{
		if (Cast<AInteractable>(m_Focus) != nullptr)
		{
			Cast<AInteractable>(m_Focus)->Interact(this);
		}
		else
		{
			// exit action
			m_InitialState.tile = startState.tile;
			m_InitialState.actionState = endState.actionState;
			m_CurrentAction.actionType = Function::NullAction;
			m_Directive = Directive::DoNothing;
		}
		return;
	}
	// exit action
	m_InitialState.tile = startState.tile;
	m_InitialState.actionState = endState.actionState;
	m_CurrentAction.actionType = Function::NullAction;
	m_Directive = Directive::DoNothing;

}


void ANPC::CallAction(Action action)
{
	// all functions are closed loops that will run until action is complete
	switch (action.actionType)
	{
	case Function::MoveFunction:
		MovePath(m_CurrentPath); // action is complete when end state is either reached or deemed impossible to get to
		break;
	case Function::AttackFunction:
		Attack(action.startingState, action.endState); // action is complete when an attack is launched at the focus or deemed impossible to attack or focus changes
		break;
	case Function::InteractFunction:
		Interact(action.startingState, action.endState);
	default:
		// do nothing
		break;
	}
}

ATile3D* ANPC::FindClosestTile(FVector2D location)
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

