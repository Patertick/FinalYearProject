// Fill out your copyright notice in the Description page of Project Settings.


#include "ResearcherGoalCreator.h"
#include <Kismet/GameplayStatics.h>

State UResearcherGoalCreator::CreateGoal()
{
	if (m_NPCRef->GetAlertness() == AlertLevel::High)
	{
		// is fearless?
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Fearless))
		{
			// yes, fight enemy
			TArray<ANPC*> threats;
			for (AActor* actor : m_NPCRef->GetCharactersInView())
			{
				if (Cast<ANPC>(actor) != nullptr)
				{
					if (Cast<ANPC>(actor)->m_Threat) // this is an enemy
					{
						threats.Add(Cast<ANPC>(actor));
					}
				}
			}
			// is enemy in view?
			if (threats.Num() <= 0)
			{
				// no, call search actions

				State newState;
				newState.tile = m_NPCRef->CallFindClosestTile(m_NPCRef->CallGetLastSeenTileLocation());
				newState.actionState = ActionState::Searching;
				return newState;
			}
			else {
				// yes, call attack actions

				// choose closest enemy, find tile, set goal state to attacking this tile

				ANPC* closestEnemy = threats[0];

				for (ANPC* npc : threats)
				{
					float oldDistance = FVector::Distance(m_NPCRef->GetActorLocation(), closestEnemy->GetActorLocation());
					float newDistance = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
					if (newDistance < oldDistance)
					{
						closestEnemy = npc;
					}
				}
				State newState;
				FVector2D enemyLocation = FVector2D{ closestEnemy->GetActorLocation().X, closestEnemy->GetActorLocation().Y };
				newState.tile = closestEnemy->CallFindClosestTile(enemyLocation);
				newState.actionState = ActionState::Attacking;
				return newState;
			}
		}
		else
		{
			// no, flee (increase speed if this NPC has the coward quality)
				// is enemy in memory?
			TArray<ANPC*> threats;
			for (ActorSnapShot actor : m_NPCRef->GetObjectsFromMemory())
			{
				if (actor.objectDesignation == ObjectType::Character)
				{
					if (Cast<ANPC>(actor.objectRef)->m_Threat) // this is an enemy
					{
						threats.Add(Cast<ANPC>(actor.objectRef));
					}
				}
			}
			// is enemy in view?
			if (threats.Num() <= 0)
			{
				// no, simply run in random direction
				State newState;
				newState.tile = m_NPCRef->CallFindClosestTile(FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
				newState.actionState = ActionState::RunningAway;
				return newState;
			}
			else {
				// yes, flee from all known enemies

				ANPC* farthestEnemy = threats[0];
				for (ANPC* npc : threats)
				{
					float oldDistance = FVector::Distance(m_NPCRef->GetActorLocation(), farthestEnemy->GetActorLocation());
					float newDistance = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
					if (newDistance > oldDistance)
					{
						farthestEnemy = npc;
					}
				}

				// get average forward vector of all enemies, giving higher weights to closer enemies
				float maxDistance = FVector::Distance(m_NPCRef->GetActorLocation(), farthestEnemy->GetActorLocation()); // used to normalise distances
				FVector2D averageForwardVector2D{ 0.0f };
				for (ANPC* npc : threats)
				{
					float modifier = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
					modifier = 1.0f - (modifier / maxDistance); // should be between 0.0 and 1.0, the closer an enemy is to the player, the more the NPC tries to escape them
					averageForwardVector2D += (FVector2D{ npc->GetActorForwardVector().X, npc->GetActorForwardVector().Y } * modifier);
				}

				averageForwardVector2D /= threats.Num(); // normalise
				
				// if zero vector or close, run in random direction
				if (averageForwardVector2D.IsNearlyZero())
				{
					State newState;
					newState.tile = m_NPCRef->CallFindClosestTile(FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
					newState.actionState = ActionState::RunningAway;
					return newState;
				}
				else {
					// if not a zero vector, reverse the vector, then find the adjacent floor tile in that direction
					averageForwardVector2D *= -1.0f;
					ATile3D* currentTile = m_NPCRef->CallFindClosestTile(FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
					ATile3D* tileInDirectionOfVector{ nullptr };
					for (ATile3D* adjacentTile : currentTile->GetConnectedTiles())
					{
						if (adjacentTile->GetType() == TileType::None)
						{
							if (tileInDirectionOfVector == nullptr)
							{
								tileInDirectionOfVector = adjacentTile;
							}
							else
							{
								// when the NPC is moved in the calculated location, which tile is closest to this new location
								FVector2D nextLocation = FVector2D{ m_NPCRef->GetActorLocation().X + (averageForwardVector2D.X * 10.0f), 
								m_NPCRef->GetActorLocation().Y + (averageForwardVector2D.Y * 10.0f) };
								float oldDistance = FVector2D::Distance(nextLocation, FVector2D{ tileInDirectionOfVector->GetActorLocation().X, tileInDirectionOfVector->GetActorLocation().Y });
								float newDistance = FVector2D::Distance(nextLocation, FVector2D{ adjacentTile->GetActorLocation().X, adjacentTile->GetActorLocation().Y });
								if (newDistance < oldDistance)
								{
									tileInDirectionOfVector = adjacentTile;
								}
							}
						}
					}

					State newState;
					newState.tile = tileInDirectionOfVector;
					newState.actionState = ActionState::RunningAway;
					return newState;
				}
				
			}		
					
		}
	}
	else if (m_NPCRef->GetAlertness() == AlertLevel::Medium)
	{
	// is fearless?
	if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Fearless))
	{
		// yes, fight enemy
		TArray<ANPC*> threats;
		for (AActor* actor : m_NPCRef->GetCharactersInView())
		{
			if (Cast<ANPC>(actor) != nullptr)
			{
				if (Cast<ANPC>(actor)->m_Threat) // this is an enemy
				{
					threats.Add(Cast<ANPC>(actor));
				}
			}
		}
		// is enemy in view?
		if (threats.Num() <= 0)
		{
			// no, call search actions

			State newState;
			newState.tile = m_NPCRef->CallFindClosestTile(m_NPCRef->CallGetLastSeenTileLocation());
			newState.actionState = ActionState::Searching;
			return newState;
		}
		else {
			// yes, call attack actions

			// choose closest enemy, find tile, set goal state to attacking this tile

			ANPC* closestEnemy = threats[0];

			for (ANPC* npc : threats)
			{
				float oldDistance = FVector::Distance(m_NPCRef->GetActorLocation(), closestEnemy->GetActorLocation());
				float newDistance = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
				if (newDistance < oldDistance)
				{
					closestEnemy = npc;
				}
			}
			State newState;
			FVector2D enemyLocation = FVector2D{ closestEnemy->GetActorLocation().X, closestEnemy->GetActorLocation().Y };
			newState.tile = closestEnemy->CallFindClosestTile(enemyLocation);
			newState.actionState = ActionState::Attacking;
			return newState;
		}
	}
	else
	{
		// no, flee (increase speed if this NPC has the coward quality)
			// is enemy in memory?
		TArray<ANPC*> threats;
		for (ActorSnapShot actor : m_NPCRef->GetObjectsFromMemory())
		{
			if (actor.objectDesignation == ObjectType::Character)
			{
				if (Cast<ANPC>(actor.objectRef)->m_Threat) // this is an enemy
				{
					threats.Add(Cast<ANPC>(actor.objectRef));
				}
			}
		}
		// is enemy in view?
		if (threats.Num() <= 0)
		{
			// no, run towards exit
			TArray<AActor*> Tiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);
			TArray<ATile3D*> possibleTiles;
			for (AActor* tile : Tiles)
			{
				if (Cast<ATile3D>(tile)->m_FloorType == FloorType::ReceptionFloor)
				{
					possibleTiles.Add(Cast<ATile3D>(tile));
				}
			}

			int32 randomTileIndex = FMath::RandRange(0, possibleTiles.Num() - 1);

			State newState;
			newState.tile = Cast<ATile3D>(possibleTiles[randomTileIndex]);
			newState.actionState = ActionState::MovingToLocation;
			return newState;
		}
		else {
			// yes, flee from all known enemies

			ANPC* farthestEnemy = threats[0];
			for (ANPC* npc : threats)
			{
				float oldDistance = FVector::Distance(m_NPCRef->GetActorLocation(), farthestEnemy->GetActorLocation());
				float newDistance = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
				if (newDistance > oldDistance)
				{
					farthestEnemy = npc;
				}
			}

			// get average forward vector of all enemies, giving higher weights to closer enemies
			float maxDistance = FVector::Distance(m_NPCRef->GetActorLocation(), farthestEnemy->GetActorLocation()); // used to normalise distances
			FVector2D averageForwardVector2D{ 0.0f };
			for (ANPC* npc : threats)
			{
				float modifier = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
				modifier = 1.0f - (modifier / maxDistance); // should be between 0.0 and 1.0, the closer an enemy is to the player, the more the NPC tries to escape them
				averageForwardVector2D += (FVector2D{ npc->GetActorForwardVector().X, npc->GetActorForwardVector().Y } *modifier);
			}

			averageForwardVector2D /= threats.Num(); // normalise

			// if zero vector or close, run in random direction
			if (averageForwardVector2D.IsNearlyZero())
			{
				State newState;
				newState.tile = m_NPCRef->CallFindClosestTile(FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
				newState.actionState = ActionState::RunningAway;
				return newState;
			}
			else {
				// if not a zero vector, reverse the vector, then find the adjacent floor tile in that direction
				averageForwardVector2D *= -1.0f;
				ATile3D* currentTile = m_NPCRef->CallFindClosestTile(FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y });
				ATile3D* tileInDirectionOfVector{ nullptr };
				for (ATile3D* adjacentTile : currentTile->GetConnectedTiles())
				{
					if (adjacentTile->GetType() == TileType::None)
					{
						if (tileInDirectionOfVector == nullptr)
						{
							tileInDirectionOfVector = adjacentTile;
						}
						else
						{
							// when the NPC is moved in the calculated location, which tile is closest to this new location
							FVector2D nextLocation = FVector2D{ m_NPCRef->GetActorLocation().X + (averageForwardVector2D.X * 10.0f),
							m_NPCRef->GetActorLocation().Y + (averageForwardVector2D.Y * 10.0f) };
							float oldDistance = FVector2D::Distance(nextLocation, FVector2D{ tileInDirectionOfVector->GetActorLocation().X, tileInDirectionOfVector->GetActorLocation().Y });
							float newDistance = FVector2D::Distance(nextLocation, FVector2D{ adjacentTile->GetActorLocation().X, adjacentTile->GetActorLocation().Y });
							if (newDistance < oldDistance)
							{
								tileInDirectionOfVector = adjacentTile;
							}
						}
					}
				}

				State newState;
				newState.tile = tileInDirectionOfVector;
				newState.actionState = ActionState::RunningAway;
				return newState;
			}

		}

	}
	}
	else if (m_NPCRef->GetAlertness() == AlertLevel::Low)
	{
		// random decision

		float workChance{ 1.0f }, takeBreakChance{ 1.0f }, meetingRoomChance{ 1.0f };

		// patrol modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Smart))
		{
			workChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Efficient))
		{
			workChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Lazy))
		{
			workChance *= 0.5f; // decrease chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Stupid))
		{
			workChance *= 0.5f; // decrease chance by 0.5
		}

		// take break modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Lazy))
		{
			takeBreakChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Stupid))
		{
			takeBreakChance *= 0.5f; // decrease chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Efficient))
		{
			takeBreakChance *= 0.5f; // decrease chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Active))
		{
			takeBreakChance *= 0.5f; // decrease chance by 0.5
		}

		// follow individual modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Charismatic))
		{
			meetingRoomChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Efficient))
		{
			meetingRoomChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Stupid))
		{
			meetingRoomChance *= 0.5f; // decrease chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Lazy))
		{
			meetingRoomChance *= 0.5f; // decrease chance by 0.5
		}

		float randomFloat = FMath::FRandRange(0.0f, workChance + takeBreakChance + meetingRoomChance);

		if (randomFloat >= 0.0f && randomFloat <= workChance)
		{
			// Go to work (go to research room or an office) (smart OR efficient +, lazy OR stupid -)
			TArray<AActor*> Tiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);
			TArray<ATile3D*> possibleTiles;
			for (AActor* tile : Tiles)
			{
				if (Cast<ATile3D>(tile)->m_FloorType == FloorType::ResearchRoomFloor)
				{
					possibleTiles.Add(Cast<ATile3D>(tile));
				}
				if (Cast<ATile3D>(tile)->m_FloorType == FloorType::OfficeFloor)
				{
					possibleTiles.Add(Cast<ATile3D>(tile));
				}
			}

			int32 randomTileIndex = FMath::RandRange(0, possibleTiles.Num() - 1);

			State newState;
			newState.tile = Cast<ATile3D>(possibleTiles[randomTileIndex]);
			newState.actionState = ActionState::MovingToLocation;
			return newState;
		}
		else if (randomFloat > workChance && randomFloat <= takeBreakChance + workChance)
		{
			// Go to break room (lazy OR stupid +, active OR efficient -)

			TArray<AActor*> Tiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);
			TArray<ATile3D*> possibleTiles;
			for (AActor* tile : Tiles)
			{
				if (Cast<ATile3D>(tile)->m_FloorType == FloorType::BreakRoomFloor)
				{
					possibleTiles.Add(Cast<ATile3D>(tile));
				}
			}

			int32 randomTileIndex = FMath::RandRange(0, possibleTiles.Num() - 1);

			State newState;
			newState.tile = Cast<ATile3D>(possibleTiles[randomTileIndex]);
			newState.actionState = ActionState::MovingToLocation;
			return newState;
		}
		else
		{
			// Go to meeting room (charismatic OR efficient +, stupid OR lazy -)

			TArray<AActor*> Tiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);
			TArray<ATile3D*> possibleTiles;
			for (AActor* tile : Tiles)
			{
				if (Cast<ATile3D>(tile)->m_FloorType == FloorType::MeetingRoomFloor)
				{
					possibleTiles.Add(Cast<ATile3D>(tile));
				}
			}

			int32 randomTileIndex = FMath::RandRange(0, possibleTiles.Num() - 1);

			State newState;
			newState.tile = Cast<ATile3D>(possibleTiles[randomTileIndex]);
			newState.actionState = ActionState::MovingToLocation;
			return newState;
		}		
	}
	return State();
}