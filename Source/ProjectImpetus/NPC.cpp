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

	m_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPC Mesh"));
	SetRootComponent(m_Mesh);
}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();
		

	m_PlanningBrain->GenerateStartingValues();

	if (m_ShouldGenerateName)
	{
		GenerateName();
	}
	if (m_ShouldGenerateMesh)
	{
		m_MeshGen->CreateNewMesh(m_Mesh, m_Material);
	}
}

Path ANPC::CreateMovePath(ATile3D* startTile, ATile3D* endTile)
{
	// set appropriate move properties
	m_WalkSpeed = m_ActionManager->GetMobilityAction().speed;
	m_CanBeTargeted = m_ActionManager->GetMobilityAction().isTargetable;
	m_CanUseActions = m_ActionManager->GetMobilityAction().canOtherActionsBeUsed;

	Path newPath;
	// create A* path to end tile
	newPath = m_PlanningBrain->FindAStarPath(startTile, endTile);
	

	return newPath;
}

void ANPC::CreateAttack(ATile3D* startTile)
{
	if (m_HasDied) return;
	float distance = FVector2D::Distance(FVector2D{ startTile->GetActorLocation().X, startTile->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
	// is the start tile within attack range?
	if (distance < m_AttackRange)
	{

		TArray<AActor*> NPC;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPC);
		ANPC* closestNPC{ nullptr };
		TArray<ANPC*> reachedNPC;
		// create a path between all enemies in range
		TArray<ATile3D*> attackTiles;

		// find closest enemy
		for (AActor* actor : NPC)
		{
			if (Cast<ANPC>(actor)->m_Threat != m_Threat && !reachedNPC.Contains(Cast<ANPC>(actor)))
			{
				if (closestNPC == nullptr)
				{
					closestNPC = Cast<ANPC>(actor);
				}
				else
				{
					float newTileDistance = FVector2D::Distance(FVector2D{ actor->GetActorLocation().X, actor->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					float oldTileDistance = FVector2D::Distance(FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					if (newTileDistance < oldTileDistance)
					{
						closestNPC = Cast<ANPC>(actor);
					}
				}
			}
		}

		if (closestNPC == nullptr) return;

		attackTiles.Add(startTile);

		for (int i = 0; i < m_ActionManager->GetOffensiveAction().numberOfTargetableTiles; i++)
		{
			// is the current location the same as current enemy?
			if (FVector2D{ attackTiles[attackTiles.Num() - 1]->GetActorLocation().X, attackTiles[attackTiles.Num() - 1]->GetActorLocation().Y }.Equals(FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y }))
			{
				// add to reached npcs
				reachedNPC.Add(closestNPC);
				if (reachedNPC.Num() >= m_PlanningBrain->GetNumberOfEnemies())
				{
					break;
				}
			}
			else
			{
				// path towards this enemy
				if (attackTiles.Num() <= 0)
				{
					attackTiles.Add(m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y }));
				}
				else
				{
					// find current tile
					ATile3D* currentTile = attackTiles[attackTiles.Num() - 1];
					// find next tile on the way to the closest enemy
					ATile3D* closestToEnemy{ nullptr };
					for (ConnectedTile adjacentTile : currentTile->GetConnectedTiles())
					{
						if (closestToEnemy == nullptr)
						{
							closestToEnemy = adjacentTile.ref;
						}
						else
						{
							float newTileDistance = FVector2D::Distance(FVector2D{ adjacentTile.ref->GetActorLocation().X, adjacentTile.ref->GetActorLocation().Y }, FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y });
							float oldTileDistance = FVector2D::Distance(FVector2D{ closestToEnemy->GetActorLocation().X, closestToEnemy->GetActorLocation().Y }, FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y });
							if (newTileDistance < oldTileDistance)
							{
								closestToEnemy = adjacentTile.ref;
							}
						}
					}

					if(closestToEnemy != nullptr) attackTiles.Add(closestToEnemy);
				}


			}
		}
		// cascade tiles towards self tile to create area attack effects

		//ATile3D* NPCTile = m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y });
		//// for every tile in the set
		//for (ATile3D* tile : attackTiles)
		//{
		//	// find the path between self tile and this tile
		//	Path path = m_PlanningBrain->FindAStarPath(tile, NPCTile);
		//	if (path.totalCost < 0)
		//	{
		//		continue;
		//	}
		//	else
		//	{
		//		// add every tile not in attackTiles
		//		for (FVector2D loc : path.locations)
		//		{
		//			if (!attackTiles.Contains(m_PlanningBrain->FindClosestTile(loc)))
		//			{
		//				attackTiles.Add(m_PlanningBrain->FindClosestTile(loc));
		//			}
		//		}
		//	}			
		//}

		for (ATile3D* tile : attackTiles)
		{
			tile->AttackTile(this);
		}
	}
}

ATile3D* ANPC::FindClosestTileToActor(AActor* actor)
{
	ATile3D* tile = m_PlanningBrain->FindClosestTile(FVector2D{ actor->GetActorLocation().X, actor->GetActorLocation().Y });

	ATile3D* closestTile{ nullptr };

	for (ConnectedTile adjacentTile : tile->GetConnectedTiles())
	{
		if (closestTile == nullptr)
		{
			closestTile = adjacentTile.ref;
		}
		else
		{
			float oldDistance = FVector2D::Distance(FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y }, FVector2D{ actor->GetActorLocation().X, actor->GetActorLocation().Y });
			float newDistance = FVector2D::Distance(FVector2D{ adjacentTile.ref->GetActorLocation().X, adjacentTile.ref->GetActorLocation().Y }, FVector2D{ actor->GetActorLocation().X, actor->GetActorLocation().Y });
			if (newDistance < oldDistance)
			{
				closestTile = adjacentTile.ref;
			}
		}
	}
	return closestTile;
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_DeltaTime = DeltaTime;
	
	if (m_HasDied)
	{
		if (m_DeathTimer >= 0.0f)
		{
			m_DeathTimer -= DeltaTime;
			return;
		}
		else
		{
			Respawn();
		}
	}

	if (!ValidNPC()) return;

	if (CallFindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y })->GetType() != TileType::None)
	{
		CallFindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y })->TurnToFloor();
	}


	if (m_CurrentTile == nullptr)
	{
		m_CurrentTile = m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y });
	}
	else
	{
		ATile3D* closestTile = m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y });
		if (closestTile != m_CurrentTile)
		{
			closestTile->SetNPCOnTile(this);
			closestTile->SetType(TileType::NPC);
			m_CurrentTile->SetNPCOnTile(nullptr);
			m_CurrentTile->SetType(TileType::None);
			m_CurrentTile = closestTile;
		}
	}

	// cooldown for taking damage
	if (!m_CanBeTargeted)
	{
		if (m_DamageTimer >= 0.0f)
		{
			m_DamageTimer -= DeltaTime;
		}
		else
		{
			m_CanBeTargeted = true;
			m_DamageTimer = KMAXDAMAGETIMER;
		}
	}


	/*if (!m_Threat)
	{
		FString tempString = FString::SanitizeFloat(m_Health);
		GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::White, *tempString);
	}*/

	if (m_Health <= 0.0f)
	{
		m_DeathTimer = KMAXDEATHTIMER;
		m_HasDied = true;
		Death();
		return;
	}

	m_Controllable = !m_Threat;	


	CallAction(m_CurrentAction);

	SetRotation();

	if (m_Threat)
	{
		m_WalkSpeed = 15.0f;
	}
	else
	{
		m_WalkSpeed = 15.0f;
	}

}

void ANPC::Death()
{
	this->SetActorEnableCollision(false);
	this->SetActorHiddenInGame(true);
	m_CurrentTile->SetType(TileType::None);
	m_CurrentTile->SetNPCOnTile(nullptr);
}

void ANPC::Respawn()
{
	m_HasDied = false;
	this->SetActorEnableCollision(true);
	this->SetActorHiddenInGame(false);
	m_Health = m_MaxHealth;
	// spawn in random tile

	TArray<AActor*> tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), tiles);

	TArray<ATile3D*> possibleTiles;

	if (m_Threat)
	{
		for (int i = 0; i < tiles.Num(); i++)
		{
			// remove non cell tiles
			if (Cast<ATile3D>(tiles[i])->m_FloorType == FloorType::CellFloor && Cast<ATile3D>(tiles[i])->GetType() == TileType::None)
			{
				possibleTiles.Add(Cast<ATile3D>(tiles[i]));
			}
		}
	}
	else
	{
		for (int i = 0; i < tiles.Num(); i++)
		{
			// remove non reception tiles
			if (Cast<ATile3D>(tiles[i])->m_FloorType == FloorType::ReceptionFloor && Cast<ATile3D>(tiles[i])->GetType() == TileType::None)
			{
				possibleTiles.Add(Cast<ATile3D>(tiles[i]));
			}
		}
	}

	m_CurrentTile = Cast<ATile3D>(possibleTiles[FMath::RandRange(0, possibleTiles.Num() - 1)]);

	m_CurrentTile->SetType(TileType::NPC);
	m_CurrentTile->SetNPCOnTile(this);

	SetActorLocation(FVector{ m_CurrentTile->GetActorLocation().X, m_CurrentTile->GetActorLocation().Y, GetActorLocation().Z });
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


void ANPC::UseAction(ActionState action)
{
	if (m_HasDied) return;

	if (action != ActionState::Searching) m_SearchTargetTile = nullptr; // if not searching, reset target tile

	// actions can change during execution, thus each step, consider what current action is in use
	// to accomplish this, the UseAction parameter will be switched, computed into an actual action (or goal)
	// then the next logical step for the AI to accomplish it's chosen action will be found and then executed
	if (action == ActionState::Attacking) // find closest enemy and attack its tile
	{
		ANPC* closestEnemy = FindClosestNPC(false, true);

		if (closestEnemy == nullptr)
		{
			return;
		}

		if (FVector2D::Distance(FVector2D{ closestEnemy->GetActorLocation().X, closestEnemy->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y }) <= m_AttackRange)
		{
			// attack closest enemy
			CreateAttack(m_PlanningBrain->FindClosestTile(FVector2D{ closestEnemy->GetActorLocation().X, closestEnemy->GetActorLocation().Y }));
		}
		else
		{
			if (m_ActionManager->GetMobilityAction().speed <= 0.0f)
			{
				// teleport to final tile
				SetActorLocation(FVector{ closestEnemy->GetActorLocation().X, closestEnemy->GetActorLocation().Y, GetActorLocation().Z });
			}
			else
			{
				// find closest adjacent tile to enemy

				ATile3D* closestAdjacentTile = FindClosestTileToActor(closestEnemy);

				// move towards closest enemy
				Path movePath = CreateMovePath(m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y }), closestAdjacentTile);
				// move towards first flag of the move path
				if (movePath.locations.Num() <= 0) return;
				FVector2D moveDir = movePath.locations[0] - FVector2D{ GetActorLocation().X, GetActorLocation().Y };
				moveDir.Normalize();
				FVector newPos;
				newPos.X = GetActorLocation().X + (moveDir.X * m_WalkSpeed * m_DeltaTime * 10.0f);
				newPos.Y = GetActorLocation().Y + (moveDir.Y * m_WalkSpeed * m_DeltaTime * 10.0f);
				SetActorLocation(FVector{ newPos.X, newPos.Y, GetActorLocation().Z });
			}
		}
	}
	else if (action == ActionState::Interacting) // find the closest interactable and interact with it
	{
		AInteractable* closestInteractable = FindClosestInteractable();

		if (closestInteractable == nullptr) return;

		if (FVector2D::Distance(FVector2D{ closestInteractable->GetActorLocation().X, closestInteractable->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y }) <= ATile3D::TileSize())
		{
			closestInteractable->Interact(this);
		}
		else
		{
			if (m_ActionManager->GetMobilityAction().speed <= 0.0f)
			{
				// teleport to final tile
				SetActorLocation(FVector{ closestInteractable->GetActorLocation().X, closestInteractable->GetActorLocation().Y, GetActorLocation().Z });
			}
			else
			{
				ATile3D* closestAdjacentTile = FindClosestTileToActor(closestInteractable);
				Path movePath = CreateMovePath(m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y }), closestAdjacentTile);
				if (movePath.locations.Num() <= 0) return;
				FVector2D moveDir = movePath.locations[0] - FVector2D{ GetActorLocation().X, GetActorLocation().Y };
				moveDir.Normalize();
				FVector newPos;
				newPos.X = GetActorLocation().X + (moveDir.X * m_WalkSpeed * m_DeltaTime * 10.0f);
				newPos.Y = GetActorLocation().Y + (moveDir.Y * m_WalkSpeed * m_DeltaTime * 10.0f);
				SetActorLocation(FVector{ newPos.X, newPos.Y, GetActorLocation().Z });
			}
		}
	}
	else if (action == ActionState::Searching) // move in a random direction (since this function is completely stochastic it should not override other actions)
	{
		// is a target tile set?
		if (m_SearchTargetTile == nullptr)
		{
			// find tile in random direction that is 1: in move range & 2: a floor tile
			TArray<AActor*> tiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), tiles);

			TArray<ATile3D*> possibleTargetTiles;

			for (AActor* tile : tiles)
			{
				if (m_ActionManager->GetMobilityAction().rangeOfMovement < 0)
				{
					// infinite range
					if (Cast<ATile3D>(tile)->GetType() == TileType::None)
					{
						possibleTargetTiles.Add(Cast<ATile3D>(tile));
					}
				}
				else
				{
					// limited range
					if (FVector2D::Distance(FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y }) <= m_ActionManager->GetMobilityAction().rangeOfMovement)
					{
						if (Cast<ATile3D>(tile)->GetType() == TileType::None)
						{
							possibleTargetTiles.Add(Cast<ATile3D>(tile));
						}
					}
				}
			}

			// pick random tile as new search target

			if (possibleTargetTiles.Num() <= 0) return;

			m_SearchTargetTile = possibleTargetTiles[FMath::RandRange(0, possibleTargetTiles.Num() - 1)];

		}
		else
		{
			// has target tile been reached?
			if (m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y }) == m_SearchTargetTile)
			{
				m_SearchTargetTile = nullptr;
			}
			else
			{
				if (m_ActionManager->GetMobilityAction().speed <= 0.0f)
				{
					// teleport to final tile
					SetActorLocation(FVector{ m_SearchTargetTile->GetActorLocation().X, m_SearchTargetTile->GetActorLocation().Y, GetActorLocation().Z });
				}
				else
				{
					Path movePath = CreateMovePath(m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y }), m_SearchTargetTile);
					if (movePath.locations.Num() <= 0) return;
					FVector2D moveDir = movePath.locations[0] - FVector2D{ GetActorLocation().X, GetActorLocation().Y };
					moveDir.Normalize();
					FVector newPos;
					newPos.X = GetActorLocation().X + (moveDir.X * m_WalkSpeed * m_DeltaTime * 10.0f);
					newPos.Y = GetActorLocation().Y + (moveDir.Y * m_WalkSpeed * m_DeltaTime * 10.0f);
					SetActorLocation(FVector{ newPos.X, newPos.Y, GetActorLocation().Z });
				}
			}
		}
	}
	else if (action == ActionState::Following) // find closest ally, move towards closest ally
	{
		// find closest ally
		ANPC* closestAlly = FindClosestNPC(true, false);
		// move towards ally
		if (m_ActionManager->GetMobilityAction().speed <= 0.0f)
		{
			// teleport to final tile
			SetActorLocation(FVector{ closestAlly->GetActorLocation().X, closestAlly->GetActorLocation().Y, GetActorLocation().Z });
		}
		else
		{
			ATile3D* closestAdjacentTile = FindClosestTileToActor(closestAlly);
			Path movePath = CreateMovePath(m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y }), closestAdjacentTile);
			if (movePath.locations.Num() <= 0) return;
			FVector2D moveDir = movePath.locations[0] - FVector2D{ GetActorLocation().X, GetActorLocation().Y };
			moveDir.Normalize();
			FVector newPos;
			newPos.X = GetActorLocation().X + (moveDir.X * m_WalkSpeed * m_DeltaTime * 10.0f);
			newPos.Y = GetActorLocation().Y + (moveDir.Y * m_WalkSpeed * m_DeltaTime * 10.0f);
			SetActorLocation(FVector{ newPos.X, newPos.Y, GetActorLocation().Z });
		}
	}
	else if (action == ActionState::RunningAway) // find all enemy positions within sensory range, find a direction they aren't in, move towards that direction
	{
		// find position of every enemy
		TArray<AActor*> NPCs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);

		TArray<ANPC*> enemies;

		for (AActor* npc : NPCs)
		{
			if (Cast<ANPC>(npc)->m_Threat != m_Threat)
			{
				enemies.Add(Cast<ANPC>(npc));
			}
		}

		if (enemies.Num() <= 0) return;

		// for each adjacent tile to the one this NPC is on
		// find the tile such that the average distance from enemies is highest

		ATile3D* idealTile{ nullptr };

		for (ConnectedTile tile : m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y })->GetConnectedTiles())
		{
			if (idealTile == nullptr)
			{
				idealTile = tile.ref;
			}
			else
			{
				// find average distance from enemy
				float newAverageDistance{ 0.0f };
				float oldAverageDistance{ 0.0f };
				for (ANPC* enemy : enemies)
				{
					newAverageDistance += FVector2D::Distance(FVector2D{ enemy->GetActorLocation().X, enemy->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					oldAverageDistance += FVector2D::Distance(FVector2D{ enemy->GetActorLocation().X, enemy->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
				}

				newAverageDistance /= enemies.Num();
				oldAverageDistance /= enemies.Num();

				if (newAverageDistance > oldAverageDistance)
				{
					idealTile = tile.ref;
				}
			}

		}

		if (idealTile == nullptr) return;


		if (m_ActionManager->GetMobilityAction().speed <= 0.0f)
		{
			// teleport to final tile
			SetActorLocation(FVector{ idealTile->GetActorLocation().X, idealTile->GetActorLocation().Y, GetActorLocation().Z });
		}
		else
		{
			// move towards this tile
			Path movePath = CreateMovePath(m_PlanningBrain->FindClosestTile(FVector2D{ GetActorLocation().X, GetActorLocation().Y }), idealTile);
			if (movePath.locations.Num() <= 0) return;
			FVector2D moveDir = movePath.locations[0] - FVector2D{ GetActorLocation().X, GetActorLocation().Y };
			moveDir.Normalize();
			FVector newPos;
			newPos.X = GetActorLocation().X + (moveDir.X * m_WalkSpeed * m_DeltaTime * 10.0f);
			newPos.Y = GetActorLocation().Y + (moveDir.Y * m_WalkSpeed * m_DeltaTime * 10.0f);
			SetActorLocation(FVector{ newPos.X, newPos.Y, GetActorLocation().Z });
		}
	}
	else if (action == ActionState::UsingAbility) // first, find what ability this NPC has, then compute the best place to use it (e.g. heal ability, heal closest injured ally)
	{

	}
	else if (action == ActionState::DoingNothing)
	{

	}
}

void ANPC::CallAction(Action action)
{
	
	
}

void ANPC::SendMessageToEmotionBrain(TPair<Emotion, float> message)
{
	m_EmotionBrain->PushMessage(message.Key, message.Value);
}

ANPC* ANPC::FindClosestNPC(bool canBeAlly, bool canBeEnemy)
{
	if (!canBeAlly && !canBeEnemy) return nullptr;

	TArray<AActor*> NPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);

	ANPC* closestNPC{ nullptr };

	for (AActor* npc : NPCs)
	{
		if (!Cast<ANPC>(npc)->GetHasDied()) // don't sense dead NPCs
		{
			if (canBeAlly && canBeEnemy)
			{
				if (closestNPC == nullptr)
				{
					closestNPC = Cast<ANPC>(npc);
				}
				else
				{
					float oldDistance = FVector2D::Distance(FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					float newDistance = FVector2D::Distance(FVector2D{ npc->GetActorLocation().X, npc->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					if (newDistance < oldDistance)
					{
						closestNPC = Cast<ANPC>(npc);
					}
				}
			}
			else if (canBeAlly && Cast<ANPC>(npc)->m_Threat == m_Threat)
			{
				if (closestNPC == nullptr)
				{
					closestNPC = Cast<ANPC>(npc);
				}
				else
				{
					float oldDistance = FVector2D::Distance(FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					float newDistance = FVector2D::Distance(FVector2D{ npc->GetActorLocation().X, npc->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					if (newDistance < oldDistance)
					{
						closestNPC = Cast<ANPC>(npc);
					}
				}
			}
			else if (canBeEnemy && Cast<ANPC>(npc)->m_Threat != m_Threat)
			{
				if (closestNPC == nullptr)
				{
					closestNPC = Cast<ANPC>(npc);
				}
				else
				{
					float oldDistance = FVector2D::Distance(FVector2D{ closestNPC->GetActorLocation().X, closestNPC->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					float newDistance = FVector2D::Distance(FVector2D{ npc->GetActorLocation().X, npc->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
					if (newDistance < oldDistance)
					{
						closestNPC = Cast<ANPC>(npc);
					}
				}
			}
		}
	}

	return closestNPC;
}

AInteractable* ANPC::FindClosestInteractable()
{
	TArray<AActor*> interactables;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractable::StaticClass(), interactables);

	AInteractable* closestInteractable{ nullptr };

	for (AActor* interactable : interactables)
	{
		if (closestInteractable == nullptr)
		{
			closestInteractable = Cast<AInteractable>(interactable);
		}
		else
		{
			float oldDistance = FVector2D::Distance(FVector2D{ closestInteractable->GetActorLocation().X, closestInteractable->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
			float newDistance = FVector2D::Distance(FVector2D{ interactable->GetActorLocation().X, interactable->GetActorLocation().Y }, FVector2D{ GetActorLocation().X, GetActorLocation().Y });
			if (newDistance < oldDistance)
			{
				closestInteractable = Cast<AInteractable>(interactable);
			}
		}
	}
	return closestInteractable;
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