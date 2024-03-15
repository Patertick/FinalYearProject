// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"
#include <Kismet/GameplayStatics.h>
#include "Math/UnrealMathUtility.h"

static int32 thisNPCIndex{ 0 };

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
	
	m_Index = thisNPCIndex;
	thisNPCIndex++;

	// set current action to null
	m_CurrentAction.actionType = Function::NullAction;
	m_CurrentAction.direction = FVector2D{ 0, 0 };
	State newState;
	FVector2D npcLocation = FVector2D{ GetActorLocation().X, GetActorLocation().Y };
	newState.tile = m_PlanningBrain->FindClosestTile(npcLocation);
	newState.tile->SetType(TileType::NPC);
	newState.actionState = ActionState::DoingNothing;
	m_PlanningBrain->SetInitialState(newState.tile, newState.actionState);
	m_PlanningBrain->AddNPCToMapStateManager(m_Index, m_Health, GetActorLocation());
		
	GenerateName();
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ValidNPC()) return;

	FString tempString = FString::SanitizeFloat(m_Health);

	if (m_Health <= 0.0f)
	{
		m_PlanningBrain->GetInitialState().tile->SetType(TileType::None);
		m_HasDied = true;
		Death();
		return;
	}

	m_SensorBrain->SetFieldOfView(m_FieldOfView);
	m_SensorBrain->SetMaxViewDistance(m_MaxViewDistance);

	m_Controllable = !m_Threat;	


	CallAction(m_CurrentAction);

	SetRotation();

}

void ANPC::Death()
{
	// has this scenario ended?
	this->SetActorEnableCollision(false);
	this->SetActorHiddenInGame(true);
	//Respawn();
}

void ANPC::Respawn()
{
	m_HasDied = false;
	m_HasEscaped = false;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Respawn"));
	this->SetActorEnableCollision(true);
	this->SetActorHiddenInGame(false);
	// set spawn tile (random reception or cell tile depending on type of NPC)
	if (m_Threat)
	{
		TArray<AActor*> Tiles;
		TArray<ATile3D*> cellTiles;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);
		for (AActor* tile : Tiles)
		{
			if (Cast<ATile3D>(tile)->m_FloorType == FloorType::CellFloor)
			{
				cellTiles.Add(Cast<ATile3D>(tile));
			}
		}

		int32 randomTile = FMath::RandRange(0, cellTiles.Num() - 1);
		// reset properties
		State newState;
		m_CurrentAction.actionType = Function::NullAction;
		FVector2D npcLocation = FVector2D{ cellTiles[randomTile]->GetActorLocation().X, cellTiles[randomTile]->GetActorLocation().Y};
		newState.tile = (Cast<ATile3D>(cellTiles[randomTile]));
		newState.tile->SetType(TileType::NPC);
		newState.actionState = ActionState::DoingNothing;
		m_PlanningBrain->SetInitialState(newState.tile, newState.actionState);
		SetActorLocation(FVector(npcLocation.X, npcLocation.Y, 0.0f));
		m_Health = m_MaxHealth;
		return;
	}
	else
	{
		TArray<AActor*> Tiles;
		TArray<ATile3D*> cellTiles;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);
		for (AActor* tile : Tiles)
		{
			if (Cast<ATile3D>(tile)->m_FloorType == FloorType::ReceptionFloor)
			{
				cellTiles.Add(Cast<ATile3D>(tile));
			}
		}

		int32 randomTile = FMath::RandRange(0, cellTiles.Num() - 1);
		// reset properties
		State newState;
		m_CurrentAction.actionType = Function::NullAction;
		FVector2D npcLocation = FVector2D{ cellTiles[randomTile]->GetActorLocation().X, cellTiles[randomTile]->GetActorLocation().Y };
		newState.tile = (Cast<ATile3D>(cellTiles[randomTile]));
		newState.tile->SetType(TileType::NPC);
		newState.actionState = ActionState::DoingNothing;
		m_PlanningBrain->SetInitialState(newState.tile, newState.actionState);
		SetActorLocation(FVector(npcLocation.X, npcLocation.Y, 0.0f));
		m_Health = m_MaxHealth;
		return;
	}
	
}

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ANPC::SetRotation()
{
	FVector newRotation;
	newRotation.X = 0;
	newRotation.Y = 0;


	if (m_CurrentAction.direction.X < -0.5f)
	{
		if (m_CurrentAction.direction.Y < -0.5f)
		{
			// left up
			newRotation.Z = 315.0f;
		}
		else if (m_CurrentAction.direction.Y > 0.5f)
		{
			// left down
			newRotation.Z = 225.0f;
		}
		else
		{
			// left
			newRotation.Z = 270.0f;
		}
	}
	else if (m_CurrentAction.direction.X > 0.5f)
	{
		if (m_CurrentAction.direction.Y < -0.5f)
		{
			// right up
			newRotation.Z = 45.0f;
		}
		else if (m_CurrentAction.direction.Y > 0.5f)
		{
			// right down
			newRotation.Z = 135.0f;
		}
		else
		{
			// right
			newRotation.Z = 90.0f;
		}
	}
	else
	{
		if (m_CurrentAction.direction.Y < -0.5f)
		{
			// up
			newRotation.Z = 0.0f;
		}
		else if (m_CurrentAction.direction.Y > 0.5f)
		{
			// down
			newRotation.Z = 180.0f;
		}
		else
		{
			// no direction, default to 0
			newRotation.Z = 0.0f;
		}
	}

	SetActorRotation(FQuat::MakeFromEuler(newRotation));
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

	UGameplayStatics::ApplyDamage(m_PlanningBrain->FindClosestNPC(FVector2D{ endState.tile->GetActorLocation().X, endState.tile->GetActorLocation().Y }), m_Damage, nullptr, this, NULL);
	State newState;
	newState.tile = startState.tile;
	newState.actionState = ActionState::DoingNothing;
	m_CurrentAction.actionType = Function::NullAction;
	return newState;
	
}

State ANPC::Interact(State startState, State endState)
{
	m_PlanningBrain->FindClosestInteractable(FVector2D{ endState.tile->GetActorLocation().X, endState.tile->GetActorLocation().Y })->Interact(this);
	State newState;
	newState.tile = startState.tile;
	newState.actionState = ActionState::DoingNothing;
	m_CurrentAction.actionType = Function::NullAction;
	return newState;

}

State ANPC::Ability(State startState, State endState)
{
	State newState;
	newState.tile = startState.tile;
	endState.tile->m_FloorType = FloorType::BrokenWallFloor; // will shrink the wall into a walkable floor tile
	endState.tile->SetType(TileType::None); // can now walk on this tile
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
	case Function::AbilityFunction:
		currentState = Ability(action.startingState, action.endState);
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

void ANPC::SendMessageToEmotionBrain(TPair<Emotion, float> message)
{
	m_EmotionBrain->PushMessage(message.Key, message.Value);
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
			// convert first letter to uppercase
			FString capitalName = generatedName.ToUpper();
			generatedName.GetCharArray()[0] = capitalName.GetCharArray()[0];
			m_Name = title + generatedName;
		}
		
	}


}


float ANPC::GetNameFitness(FString name)
{
	if (name.Len() <= 0) return 0.0f; // make sure a null name has zero fitness

	float totalFitness{ 0.0f };
	float maxFitness{ 0.0f };

	bool tooManyContinuousVowels{ false };
	bool tooManyContinuousConsonants{ false };

	int32 continuousVowels{ 0 };
	int32 continuousConsonants{ 0 };

	// measure continuous vowels / consonants for fitness (rgf makes no sense in any context)
	for (int i = 0; i < name.Len() - 1; i++)
	{
		char lastChar;
		if (i != 0)
		{
			lastChar = name.GetCharArray()[i - 1];
		}
		else
		{
			lastChar = '#';
		}
		char currentChar = name.GetCharArray()[i];
		char nextChar = name.GetCharArray()[i + 1];

		if (IsVowel(currentChar) && IsVowel(nextChar))
		{
			continuousConsonants = 0;
			continuousVowels++;
		}
		else if (!IsVowel(currentChar) && !IsVowel(nextChar))
		{
			continuousVowels = 0;
			continuousConsonants++;
		}

		if (continuousVowels > 3)
		{
			tooManyContinuousVowels = true;
		}
		else if (continuousConsonants > 2)
		{
			tooManyContinuousConsonants = true;
		}
		
		totalFitness += GetPairingValidityFitness(lastChar, currentChar, nextChar, maxFitness);
		

	}

	if (!tooManyContinuousConsonants)
	{
		totalFitness++;
	}
	if (!tooManyContinuousVowels)
	{
		totalFitness++;
	}
	maxFitness++;
	maxFitness++;

	// fitness should be between 0.0 and 1.0
	return totalFitness / maxFitness;
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
	for (int i = 97; i < 123; i++)// a-z lowercase
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


float ANPC::GetPairingValidityFitness(char first, char second, char third, float &maxFitnessRef)
{
	float fitness{ 0.0f };
	// switch look up table that finds valid pairings of characters (valid meaning reasonable for names)
	switch (second)
	{
	case 'a':
		// increase fitness if the preceding character is ideal (in this case testing for non-ideal characters)
		if (first != 'q' && first != 'u' && first != 'a')
		{
			fitness++;
		}
		// do the same for the following character
		if (third != 'e' && third != 'a' && third != 'h')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'b':
		if (first != 'q' && first != 't' && first != 'y' && first != 'p' && first != 'd' && first != 'f' && first != 'g'
			&& first != 'h' && first != 'j' && first != 'k' && first != 'z' && first != 'x' && first != 'c' && first != 'v' && first != 'b')
		{
			fitness++;
		}
		if (third != 'q' && third != 't' && third != 'p' && third != 'd' && third != 'f' && third != 'g' && third != 'j'
			&& third != 'k' && third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'c':
		if (first != 'q' && first != 'w' && first != 'r' && first != 't' && first != 'y' && first != 'p' && first != 'd'
			&& first != 'f' && first != 'g' && first != 'h' && first != 'j' && first != 'k' && first != 'l' && first != 'z' && first != 'x'
			&& first != 'c' && first != 'v' && first != 'b' && first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 't' && third != 'p' && third != 'd' && third != 'f' && third != 'g'
			&& third != 'j' && third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'd':
		if (first != 'q' && first != 't' && first != 'y' && first != 'p' && first != 'f' && first != 'g' && first != 'h'
			&& first != 'j' && first != 'k' && first != 'z' && first != 'x' && first != 'c' && first != 'v' && first != 'b')
		{
			fitness++;
		}
		if (third != 'q' && third != 't' && third != 'p' && third != 'k' && third != 'f' && third != 'g'
			&& third != 'j' && third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'e':
		if (first != 'q' && first != 'i')
		{
			fitness++;
		}
		if (third != 'w' && third != 'o' && third != 'j')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'f':
		if (first != 'q' && first != 't' && first != 'y' && first != 'p' && first != 'd'
			&& first != 'g' && first != 'h' && first != 'j' && first != 'k' && first != 'z' && first != 'x'
			&& first != 'c' && first != 'v' && first != 'b' && first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 't' && third != 'y' && third != 'p' && third != 'd' && third != 'g' && third != 'h' && third != 'k'
			&& third != 'j' && third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'g':
		if (first != 'q' && first != 't' && first != 'p' && first != 'd' && first != 'f' && first != 'h' && first != 'j'
			&& first != 'k' && first != 'z' && first != 'x' && first != 'c' && first != 'v' && first != 'b' && first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 't' && third != 'p' && third != 'd' && third != 'f' && third != 'j' && third != 'k'
			&& third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'h':
		if (first != 'q' && first != 'y' && first != 'f' && first != 'h' && first != 'j' && first != 'l' && first != 'z'
			&& first != 'x' && first != 'v')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 't' && third != 'p' && third != 's' && third != 'd' && third != 'f' && third != 'g'
			&& third != 'h' && third != 'j' && third != 'k' && third != 'l' && third != 'z' && third != 'x' && third != 'c' && third != 'v' 
			&& third != 'b' && third != 'n'	&& third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'i':
		if (first != 'q' && first != 'i')
		{
			fitness++;
		}
		if (third != 'w' && third != 'e' && third != 'a' && third != 'u' && third != 'i' && third != 'o' && third != 'h' && third != 'j')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'j':
		if (first != 'q' && first != 'w' && first != 'e' && first != 'r' && first != 't'
			&& first != 'y' && first != 'u' && first != 'i' && first != 'p' && first != 's' && first != 'd'
			&& first != 'f' && first != 'g' && first != 'h' && first != 'j' && first != 'k' && first != 'l'
			&& first != 'z' && first != 'x' && first != 'c' && first != 'v' && first != 'b' && first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 'r' && third != 't' && third != 'y' && third != 'p' && third != 's' && third != 'd'
			&& third != 'f' && third != 'g' && third != 'h' && third != 'j' && third != 'k' && third != 'l' && third != 'z' && third != 'x'
			&& third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'k':
		if (first != 'q' && first != 't' && first != 'y' && first != 'p' && first != 'd' && first != 'f' && first != 'g'
			&& first != 'h' && first != 'j' && first != 'k' && first != 'z' && first != 'x' && first != 'b')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 't' && third != 'p' && third != 'd' && third != 'f' && third != 'g' && third != 'j' && third != 'k'
			&& third != 'z' && third != 'x' && third != 'c' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'l':
		if (first != 'q' && first != 't' && first != 'y' && first != 'h' && first != 'j' && first != 'z' && first != 'x'
			&& first != 'n' && first != 'm' && first != 'v')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 'r' && third != 'y' && third != 'h' && third != 'j' && third != 'x' && third != 'c' && third != 'v')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'm':
		if (first != 'q' && first != 't' && first != 'p' && first != 'd' && first != 'f' && first != 'g'
			&& first != 'h' && first != 'j' && first != 'k' && first != 'c' && first != 'x' && first != 'v' && first != 'b' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 'r' && third != 'f' && third != 'g' && third != 'h' && third != 'j' && third != 'l'
			&& third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'n':
		if (first != 'q' && first != 't' && first != 'p' && first != 'd' && first != 'g'
			&& first != 'h' && first != 'j' && first != 'k' && first != 'c' && first != 'x' && first != 'v' && first != 'b')
		{
			fitness++;
		}
		if (third != 'q' && third != 'r' && third != 'f' && third != 'g' && third != 'h' && third != 'j' && third != 'l'
			&& third != 'z' && third != 'x' && third != 'c' && third != 'v')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'o':
		if (first != 'q' && first != 'e' && first != 'u' && first != 'i')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'p':
		if (first != 'q' && first != 't' && first != 'y' && first != 'd' && first != 'f' && first != 'g'
			&& first != 'h' && first != 'j' && first != 'k' && first != 'z' && first != 'x' && first != 'c' && first != 'v' && first != 'b')
		{
			fitness++;
		}
		if (third != 'q' && third != 't' && third != 'd' && third != 'f' && third != 'g' && third != 'j' && third != 'k'
			&& third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'q':
		if (first != 'q' && first != 'w' && first != 'r' && first != 't' && first != 'y'
			&& first != 'u' && first != 'i' && first != 'p' && first != 's' && first != 'd' && first != 'f' && first != 'g' && first != 'h'
			&& first != 'j' && first != 'k' && first != 'l' && first != 'z' && first != 'x' && first != 'c' && first != 'v' && first != 'b'
			&& first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 'e' && third != 'r' && third != 't' && third != 'y' && third != 'i'
			&& third != 'o' && third != 'p' && third != 'a' && third != 's' && third != 'd' && third != 'f' && third != 'g'
			&& third != 'h' && third != 'j' && third != 'k' && third != 'l' && third != 'z' && third != 'x' && third != 'c'
			&& third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'r':
		if (first != 'q' && first != 'r' && first != 'y' && first != 'j' && first != 'k' && first != 'l'
			&& first != 'x' && first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 'r' && third != 't' && third != 'j' && third != 'x')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 's':
		if (first != 'q' && first != 'h' && first != 'z' && first != 'x' && first != 'v' && first != 'b'
			&& first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'j' && third != 'z' && third != 'x' && third != 'v')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 't':
		if (first != 'q' && first != 'w' && first != 'y' && first != 'p' && first != 'd' && first != 'f' && first != 'g'
			&& first != 'h' && first != 'j' && first != 'k' && first != 'z' && first != 'x' && first != 'c' && first != 'v' && first != 'b')
		{
			fitness++;
		}
		if (third != 'q' && third != 'p' && third != 'd' && third != 'f' && third != 'g' && third != 'j' && third != 'k' && third != 'l'
			&& third != 'z' && third != 'x' && third != 'c' && third != 'v' && third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'u':
		if (first != 'i')
		{
			fitness++;
		}
		if (third != 'q' && third != 'o' && third != 'a' && third != 'j')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'v':
		if (first != 'q' && first != 'w' && first != 'r' && first != 't' && first != 'y' && first != 'p' && first != 's' && first != 'd'
			&& first != 'f' && first != 'g' && first != 'h' && first != 'j' && first != 'k' && first != 'l' && first != 'z' && first != 'x'
			&& first != 'c' && first != 'b' && first != 'n' && first != 'm' && first != 'v')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 't' && third != 'p' && third != 's' && third != 'd' && third != 'f' && third != 'g'
			&& third != 'h' && third != 'j' && third != 'k' && third != 'l' && third != 'z' && third != 'x' && third != 'c' && third != 'v'
			&& third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'w':
		if (first != 'q' && first != 'w' && first != 'r' && first != 'y' && first != 'h' && first != 'j' && first != 'k' && first != 'l' 
			&& first != 'z' && first != 'x' && first != 'c' && first != 'v')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 't' && third != 'j' && third != 'z' && third != 'x' && third != 'c' && third != 'v')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'x':
		if (first != 'q' && first != 'w' && first != 'r' && first != 't' && first != 'y' && first != 'p' && first != 's' && first != 'd'
			&& first != 'f' && first != 'g' && first != 'h' && first != 'j' && first != 'k' && first != 'l' && first != 'z' && first != 'x'
			&& first != 'c' && first != 'b' && first != 'n' && first != 'm' && first != 'v')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 'r' && third != 't' && third != 'y' && third != 'p' && third != 's' && third != 'd' 
			&& third != 'f' && third != 'g' && third != 'h' && third != 'j' && third != 'k' && third != 'l' && third != 'z' && third != 'x'
			&& third != 'c' && third != 'v'	&& third != 'b' && third != 'n' && third != 'm')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'y':
		if (first != 'q' && first != 'f' && first != 'j' && first != 'l' && first != 'x' && first != 'y')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 'r' && third != 't' && third != 'y' && third != 'p' && third != 'd'
			&& third != 'f' && third != 'h' && third != 'j' && third != 'k' && third != 'l' && third != 'z' && third != 'x'
			&& third != 'c' && third != 'v' && third != 'b')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	case 'z':
		if (first != 'q' && first != 'w' && first != 't' && first != 'y' && first != 'p' && first != 's' && first != 'd'
			&& first != 'f' && first != 'g' && first != 'h' && first != 'j' && first != 'k' && first != 'x' && first != 'c'
			&& first != 'v' && first != 'b' && first != 'n' && first != 'm')
		{
			fitness++;
		}
		if (third != 'q' && third != 'w' && third != 't' && third != 'p' && third != 's' && third != 'd'
			&& third != 'f' && third != 'g' && third != 'h' && third != 'j' && third != 'k' && third != 'l' && third != 'x'
			&& third != 'c' && third != 'v' && third != 'b')
		{
			fitness++;
		}
		maxFitnessRef++;
		maxFitnessRef++;
		break;
	}


	return fitness;
}