// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"
#include <Kismet/GameplayStatics.h>
#include "Math/UnrealMathUtility.h"

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


	// set current action to null
	m_CurrentAction.actionType = Function::NullAction;
	State newState;
	FVector2D npcLocation = FVector2D{ GetActorLocation().X, GetActorLocation().Y };
	newState.tile = m_PlanningBrain->FindClosestTile(npcLocation);
	newState.tile->SetType(TileType::NPC);
	newState.actionState = ActionState::DoingNothing;
	m_PlanningBrain->SetInitialState(newState.tile, newState.actionState);
		
	GenerateName();
	
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ValidNPC()) return;

	if (m_Health <= 0.0f)
	{
		m_PlanningBrain->GetInitialState().tile->SetType(TileType::None);
		Destroy(); // death
		return;
	}


	m_SensorBrain->SetFieldOfView(m_FieldOfView);
	m_SensorBrain->SetMaxViewDistance(m_MaxViewDistance);

	m_Controllable = !m_Threat;	


	CallAction(m_CurrentAction);

}

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//bool ANPC::Move(const Path &path, int32 &pointOnPath)
//{
//	// interpolation function
//	// TO DO
//	// movement
//	// TEMP
//	if (pointOnPath >= path.locations.Num() - 1)
//	{
//		// reached end of path, make current path null
//		return true;
//	}
//	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("MovePathSuccess"));
//
//	FVector2D currentPos = FVector2D{ GetActorLocation().X, GetActorLocation().Y };
//
//	FVector2D nextPoint = path.locations[pointOnPath + 1];
//
//	FVector2D endPos = nextPoint;
//	// use 2D vectors for movement as Z is constrained
//	FVector2D moveVector = (endPos - currentPos).GetSafeNormal();
//	moveVector *= m_WalkSpeed;
//	FVector newLocation = GetActorLocation() + FVector{ moveVector.X, moveVector.Y, 0.0f };
//	SetActorLocation(newLocation);
//	currentPos = FVector2D{ newLocation.X, newLocation.Y };
//
//	if (currentPos.Equals(endPos, m_WalkSpeed))
//	{
//		// successful move
//		m_InitialState.tile = FindClosestTile(nextPoint);
//		FindClosestTile(path.locations[pointOnPath])->SetType(TileType::None);
//		FindClosestTile(nextPoint)->SetType(TileType::NPC);
//		pointOnPath++;
//		SetActorLocation(FVector{ m_InitialState.tile->GetActorLocation().X, m_InitialState.tile->GetActorLocation().Y, m_HalfHeight });
//	}
//	return false;
//
//}

State ANPC::Move(State startState, State endState)
{
	// move between tiles
	if (!startState.tile->GetConnectedTiles().Contains(endState.tile) || endState.tile->GetType() != TileType::None)
	{
		State newState;
		newState.tile = startState.tile;
		newState.actionState = ActionState::DoingNothing;
		m_CurrentAction.actionType = Function::NullAction;
		return newState;
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
		State newState;
		newState.tile = endState.tile;

		SetActorLocation(FVector{ endState.tile->GetActorLocation().X, endState.tile->GetActorLocation().Y, m_HalfHeight });

		newState.actionState = ActionState::DoingNothing;
		m_CurrentAction.actionType = Function::NullAction;
		return newState;
	}
	return NULLState();
}

State ANPC::Attack(State startState, State endState)
{
	// while target on endstate is alive
	if (endState.tile->GetType() == TileType::NPC && m_PlanningBrain->GetFocus() != nullptr)
	{
		// if endstate is within range of startstate and the endstate tile type is defined as having an NPC on it, then apply damage to that npc
		UGameplayStatics::ApplyDamage(m_PlanningBrain->GetFocus(), m_Damage, nullptr, this, NULL);

		// exit action
		State newState;
		newState.tile = startState.tile; // attacking function, do not change tile position 
		newState.actionState = ActionState::Attacking; // attack has finished, reset directive
		m_CurrentAction.actionType = Function::NullAction;
		return newState;
	}
	// exit action
	State newState;
	newState.tile = startState.tile;
	newState.actionState = ActionState::DoingNothing;
	m_CurrentAction.actionType = Function::NullAction;
	return newState;
	
}

State ANPC::Interact(State startState, State endState)
{
	if (endState.tile->GetType() == TileType::Object && m_PlanningBrain->GetFocus() != nullptr)
	{
		if (Cast<AInteractable>(m_PlanningBrain->GetFocus()) != nullptr)
		{
			Cast<AInteractable>(m_PlanningBrain->GetFocus())->Interact(this);
			State newState;
			newState.tile = startState.tile;
			newState.actionState = ActionState::Interacting;
			m_CurrentAction.actionType = Function::NullAction;
			return newState;
		}
		else
		{
			// exit action
			State newState;
			newState.tile = startState.tile;
			newState.actionState = endState.actionState;
			m_CurrentAction.actionType = Function::NullAction;
			return newState;
		}
		return NULLState();
	}
	// exit action
	State newState;
	newState.tile = startState.tile;
	newState.actionState = ActionState::DoingNothing;
	m_CurrentAction.actionType = Function::NullAction;
	return newState;

}


void ANPC::CallAction(Action action)
{
	State currentState;
	// all functions are closed loops that will run until action is complete
	switch (action.actionType)
	{
	case Function::MoveFunction:
		currentState = Move(action.startingState, action.endState); // action is complete when end state is either reached or deemed impossible to get to
		if (currentState == NULLState())
		{
			// do nothing, action is still runnning
		}
		else
		{
			m_PlanningBrain->GetInitialState().tile->SetType(TileType::None);
			m_PlanningBrain->SetInitialState(currentState.tile, currentState.actionState); // action has completed and initial state updates
			m_PlanningBrain->GetInitialState().tile->SetType(TileType::NPC);
		}
		break;
	case Function::AttackFunction:
		currentState = Attack(action.startingState, action.endState); // action is complete when an attack is launched at the focus or deemed impossible to attack or focus changes
		if (currentState == NULLState())
		{
			// do nothing, action is still runnning
		}
		else
		{
			m_PlanningBrain->GetInitialState().tile->SetType(TileType::None);
			m_PlanningBrain->SetInitialState(currentState.tile, currentState.actionState); // action has completed and initial state updates
			m_PlanningBrain->GetInitialState().tile->SetType(TileType::NPC);
		}
		break;
	case Function::InteractFunction:
		currentState = Interact(action.startingState, action.endState);
		if (currentState == NULLState())
		{
			// do nothing, action is still runnning
		}
		else
		{
			m_PlanningBrain->GetInitialState().tile->SetType(TileType::None);
			m_PlanningBrain->SetInitialState(currentState.tile, currentState.actionState); // action has completed and initial state updates
			m_PlanningBrain->GetInitialState().tile->SetType(TileType::NPC);
		}
	default:
		// do nothing
		break;
	}

	
}


void ANPC::GenerateName()
{
	// generate size of name

	int32 numberOfPairs = FMath::RandRange(KNAMELENGTHMIN, KNAMELENGTHMAX);

	FString title = "";
	int32 titleNum = FMath::RandRange(0, 3);

	if (titleNum == 0)
	{
		title = "Dr. ";
	}
	else if (titleNum == 1)
	{
		title = "Officer ";
	}
	else if (titleNum == 2)
	{
		title = "Mr. ";
	}
	else if (titleNum == 3)
	{
		title = "Mrs. ";
	}

	for (int i = 0; i < KNAMEGENERATIONTHRESHOLD; i++) // exit once a valid name has been found
	{
		FString generatedName = CreateConnectorName(numberOfPairs);
		
		if (GetNameFitness(generatedName) > GetNameFitness(m_Name))
		{
			m_Name = title + generatedName;
		}
		
	}
}


float ANPC::GetNameFitness(FString name)
{
	if (name.Len() <= 0) return 0.0f; // make sure a null name has zero fitness

	// fitness should be between 0.0 and 1.0
	return 1.0f;
}

bool ANPC::IsVowel(char character)
{
	if (character == 'a' || character == 'e' || character == 'i' || character == 'o' || character == 'u')
	{
		return true;
	}
	return false;
}

TArray<FString> ANPC::GenerateStartConnectors()
{
	TArray<FString> potentialStartConnectors;
	for (int i = 65; i < 91; i++)// a-z Uppercase as this is the start letter
	{
		char newCharacter = static_cast<char>(i); 
		if (!IsVowel(newCharacter))
		{
			FString startConnector = "";
			startConnector = startConnector + newCharacter;
			startConnector = startConnector + 'a';
			potentialStartConnectors.Add(startConnector);
			startConnector = "";
			startConnector = startConnector + newCharacter;
			startConnector = startConnector + 'e';
			potentialStartConnectors.Add(startConnector);
			startConnector = "";
			startConnector = startConnector + newCharacter;
			startConnector = startConnector + 'i';
			potentialStartConnectors.Add(startConnector);
			startConnector = "";
			startConnector = startConnector + newCharacter;
			startConnector = startConnector + 'o';
			potentialStartConnectors.Add(startConnector);
			startConnector = "";
			startConnector = startConnector + newCharacter;
			startConnector = startConnector + 'u';
			potentialStartConnectors.Add(startConnector);
		}

	}

	return potentialStartConnectors;
}

TArray<FString> ANPC::GenerateMiddleConnectors()
{
	TArray<FString> potentialMiddleConnectors;
	for (int i = 97; i < 123; i++)// a-z lowercase
	{
		char newCharacter = static_cast<char>(i);
		FString middleConnector = "";
		if (!IsVowel(newCharacter))
		{
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'a';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'e';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'i';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'o';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'u';
			potentialMiddleConnectors.Add(middleConnector);
		}
		if (IsVowel(newCharacter))
		{
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'r';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 't';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'p';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 's';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'd';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'f';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'g';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'k';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'l';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'b';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'n';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'm';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'a';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'e';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'i';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'o';
			potentialMiddleConnectors.Add(middleConnector);
			middleConnector = "";
			middleConnector = middleConnector + newCharacter;
			middleConnector = middleConnector + 'u';
			potentialMiddleConnectors.Add(middleConnector);

		}
	}

	return potentialMiddleConnectors;
}

TArray<FString> ANPC::GenerateEndConnectors()
{
	TArray<FString> potentialEndConnectors;
	for (int i = 97; i < 123; i++)// a-z lowercase
	{
		char newCharacter = static_cast<char>(i);
		if (IsVowel(newCharacter))
		{
			FString endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'r';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 't';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'p';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 's';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'd';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'f';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'g';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + "sh";
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + "tch";
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'k';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + "ck";
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'l';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + "ce";
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'b';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'n';
			potentialEndConnectors.Add(endConnector);
			endConnector = "";
			endConnector = endConnector + newCharacter;
			endConnector = endConnector + 'm';
			potentialEndConnectors.Add(endConnector);

		}
	}

	return potentialEndConnectors;
}

FString ANPC::CreateConnectorName(int32 numberOfPairs)
{
	TArray<FString> startConnectors = GenerateStartConnectors();
	TArray<FString> middleConnectors = GenerateMiddleConnectors();
	TArray<FString> endConnectors = GenerateEndConnectors();
	FString name = "";
	for (int i = 0; i < numberOfPairs; i++)
	{
		if (i == 0)
		{
			name = name + startConnectors[FMath::RandRange(0, startConnectors.Num() - 1)];
		}
		else if (i == numberOfPairs - 1)
		{
			name = name + endConnectors[FMath::RandRange(0, endConnectors.Num() - 1)];
		}
		else
		{
			name = name + middleConnectors[FMath::RandRange(0, middleConnectors.Num() - 1)];
		}
	}
	return name;
}