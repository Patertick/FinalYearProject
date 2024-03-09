// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentGoalCreator.h"
#include <Kismet/GameplayStatics.h>

State UAgentGoalCreator::CreateGoal()
{
	if (m_NPCRef->GetAlertness() == AlertLevel::High)
	{
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
	else if (m_NPCRef->GetAlertness() == AlertLevel::Medium)
	{
		// random decision

		float searchAndDestroyChance{ 1.0f }, searchAndAidChance{ 1.0f }, secureRoomsChance{ 1.0f };

		// search and destroy modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Fearless))
		{
			searchAndDestroyChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Violent))
		{
			searchAndDestroyChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Coward))
		{
			searchAndDestroyChance *= 0.5f; // decrease chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Pacifist))
		{
			searchAndDestroyChance *= 0.5f; // decrease chance by 0.5
		}

		// search and aid modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::MoralCompass))
		{
			searchAndAidChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Pacifist))
		{
			searchAndAidChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Evil))
		{
			searchAndAidChance *= 0.5f; // decrease chance by 0.5
		}

		// secure rooms modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Active))
		{
			secureRoomsChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Efficient))
		{
			secureRoomsChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Lazy))
		{
			secureRoomsChance *= 0.5f; // decrease chance by 0.5
		}

		float randomFloat = FMath::FRandRange(0.0f, searchAndDestroyChance + searchAndAidChance +  secureRoomsChance);

		if (randomFloat >= 0.0f && randomFloat <= searchAndDestroyChance)
		{
			// search and contain/kill enemies (fearless OR violent +, coward OR pacifist -)
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
		else if (randomFloat > searchAndDestroyChance && randomFloat <= searchAndDestroyChance + searchAndAidChance)
		{
			// search and give aid to injured or escaping allies (moral compass OR pacifist +, evil -) 
			TArray<ANPC*> allies;
			for (AActor* actor : m_NPCRef->GetCharactersInView())
			{
				if (Cast<ANPC>(actor) != nullptr)
				{
					if (!Cast<ANPC>(actor)->m_Threat) // this is an ally
					{
						allies.Add(Cast<ANPC>(actor));
					}
				}
			}
			 
			// is ally in view?
			if (allies.Num() <= 0)
			{
				// no, call search actions
				State newState;
				newState.tile = m_NPCRef->CallFindClosestTile(m_NPCRef->CallGetLastSeenTileLocation());
				newState.actionState = ActionState::Searching;
				return newState;
			}
			else {
				// yes, call follow actions

				// find target (closest ally)

				ANPC* closestAlly = allies[0];

				for (ANPC* npc : allies)
				{
					float oldDistance = FVector::Distance(m_NPCRef->GetActorLocation(), closestAlly->GetActorLocation());
					float newDistance = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
					if (newDistance < oldDistance)
					{
						closestAlly = npc;
					}
				}

				// find tile that our target is on

				ATile3D* tile = m_NPCRef->CallFindClosestTile(FVector2D{ closestAlly->GetActorLocation().X, closestAlly->GetActorLocation().Y });
				
				// go through adjacent tiles until the closest floor tile has been found

				ATile3D* closestTile{ nullptr };

				for (ATile3D* adjacentTile : tile->GetConnectedTiles())
				{
					if (adjacentTile->GetType() == TileType::None)
					{
						if (closestTile == nullptr)
						{
							closestTile = adjacentTile;
						}
						else
						{
							FVector2D npcLoc = FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y };
							FVector2D closestTileLoc = FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y };
							FVector2D adjacentTileLoc = FVector2D{ adjacentTile->GetActorLocation().X, adjacentTile->GetActorLocation().Y };
							float oldDistance = FVector2D::Distance(npcLoc, closestTileLoc);
							float newDistance = FVector2D::Distance(npcLoc, adjacentTileLoc);
							if (newDistance < oldDistance)
							{
								closestTile = adjacentTile;
							}
						}
					}
				}

				if (closestTile == nullptr)
				{
					// couldnt find path, just head towards tile the npc is on
					State newState;
					newState.tile = tile;
					newState.actionState = ActionState::Following;
					return newState;
				}
				else
				{
					State newState;
					newState.tile = closestTile;
					newState.actionState = ActionState::Following;
					return newState;
				}
			}	
				
		}
		else {
			// Secure rooms (active OR efficient +, lazy -)
			// find last seen tile
			// move towards tile
			State newState;
			newState.tile = m_NPCRef->CallFindClosestTile(m_NPCRef->CallGetLastSeenTileLocation());
			newState.actionState = ActionState::Searching;
			return newState;
		}
		
		
		
	}
	else if (m_NPCRef->GetAlertness() == AlertLevel::Low)
	{
		// random decision

		float patrolChance{ 1.0f }, takeBreakChance{ 1.0f }, followIndividualChance{ 1.0f };

		// patrol modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Active))
		{
			patrolChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Efficient))
		{
			patrolChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Lazy))
		{
			patrolChance *= 0.5f; // decrease chance by 0.5
		}

		// take break modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Lazy))
		{
			takeBreakChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Active))
		{
			takeBreakChance *= 0.5f; // decrease chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Efficient))
		{
			takeBreakChance *= 0.5f; // decrease chance by 0.5
		}

		// follow individual modifiers
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::MoralCompass))
		{
			followIndividualChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Charismatic))
		{
			followIndividualChance *= 1.5f; // increase chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Stupid))
		{
			followIndividualChance *= 0.5f; // decrease chance by 0.5
		}
		if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Evil))
		{
			followIndividualChance *= 0.5f; // decrease chance by 0.5
		}

		float randomFloat = FMath::FRandRange(0.0f, patrolChance + takeBreakChance + followIndividualChance);

		if (randomFloat >= 0.0f && randomFloat <= patrolChance)
		{
			// patrol (active OR efficient +, lazy -)
			// find random hallway tile and move towards tile
			TArray<AActor*> Tiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);

			for (AActor* tile : Tiles)
			{
				if (Cast<ATile3D>(tile)->m_FloorType != FloorType::HallwayFloor)
				{
					Tiles.Remove(tile); // delete tiles that aren't hallway tiles
				}
			}

			int32 randomTileIndex = FMath::RandRange(0, Tiles.Num() - 1);

			State newState;
			newState.tile = Cast<ATile3D>(Tiles[randomTileIndex]);
			newState.actionState = ActionState::MovingToLocation;
			return newState;

		}
		else if (randomFloat > patrolChance && randomFloat <= takeBreakChance + patrolChance)
		{
			// take break (lazy +, active OR efficient -)
			// move towards random break room tile
			TArray<AActor*> Tiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);

			for (AActor* tile : Tiles)
			{
				if (Cast<ATile3D>(tile)->m_FloorType != FloorType::BreakRoomFloor)
				{
					Tiles.Remove(tile); // delete tiles that aren't hallway tiles
				}
			}

			int32 randomTileIndex = FMath::RandRange(0, Tiles.Num() - 1);

			State newState;
			newState.tile = Cast<ATile3D>(Tiles[randomTileIndex]);
			newState.actionState = ActionState::MovingToLocation;
			return newState;
		}
		else
		{
			// follow important individuals (moral compass OR charismatic +, stupid OR evil -)
			TArray<ANPC*> allies;
			for (AActor* actor : m_NPCRef->GetCharactersInView())
			{
				if (Cast<ANPC>(actor) != nullptr)
				{
					if (!Cast<ANPC>(actor)->m_Threat) // this is an ally
					{
						allies.Add(Cast<ANPC>(actor));
					}
				}
			}

			// is ally in view?
			if (allies.Num() <= 0)
			{
				// no, call search actions
				State newState;
				newState.tile = m_NPCRef->CallFindClosestTile(m_NPCRef->CallGetLastSeenTileLocation());
				newState.actionState = ActionState::Searching;
				return newState;
			}
			else {
				// yes, call follow actions

				// find target (closest ally)

				ANPC* closestAlly = allies[0];

				for (ANPC* npc : allies)
				{
					float oldDistance = FVector::Distance(m_NPCRef->GetActorLocation(), closestAlly->GetActorLocation());
					float newDistance = FVector::Distance(m_NPCRef->GetActorLocation(), npc->GetActorLocation());
					if (newDistance < oldDistance && (closestAlly->m_CharacterType == CharacterType::ResearcherCharacter || 
						closestAlly->m_CharacterType == CharacterType::VolunteerCharacter)) // only follow research or volunteer type characters
					{
						closestAlly = npc;
					}
				}

				// find tile that our target is on

				ATile3D* tile = m_NPCRef->CallFindClosestTile(FVector2D{ closestAlly->GetActorLocation().X, closestAlly->GetActorLocation().Y });

				// go through adjacent tiles until the closest floor tile has been found

				ATile3D* closestTile{ nullptr };

				for (ATile3D* adjacentTile : tile->GetConnectedTiles())
				{
					if (adjacentTile->GetType() == TileType::None)
					{
						if (closestTile == nullptr)
						{
							closestTile = adjacentTile;
						}
						else
						{
							FVector2D npcLoc = FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y };
							FVector2D closestTileLoc = FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y };
							FVector2D adjacentTileLoc = FVector2D{ adjacentTile->GetActorLocation().X, adjacentTile->GetActorLocation().Y };
							float oldDistance = FVector2D::Distance(npcLoc, closestTileLoc);
							float newDistance = FVector2D::Distance(npcLoc, adjacentTileLoc);
							if (newDistance < oldDistance)
							{
								closestTile = adjacentTile;
							}
						}
					}
				}

				if (closestTile == nullptr)
				{
					// couldnt find path, just head towards tile the npc is on
					State newState;
					newState.tile = tile;
					newState.actionState = ActionState::Following;
					return newState;
				}
				else
				{
					State newState;
					newState.tile = closestTile;
					newState.actionState = ActionState::Following;
					return newState;
				}
			}
		}	
	}
	return State();
}