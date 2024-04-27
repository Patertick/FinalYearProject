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

	for (AActor* tile : Tiles)
	{
		m_MapData.Add(Cast<ATile3D>(tile));
	}

	// for every possible state, create a Q Node for each possible action
	m_CurrentNode = QNode{};
}

// Called every frame
void UPlanningBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_NPCRef == nullptr) return;

	if (!m_NPCRef->GetHasDied())
	{
		// if number of allies or number of enemies changes, create new q values

		TArray<AActor*> NPCs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);
		if (NPCs.Num() != m_NumberOfAllies + m_NumberOfEnemies + 1)
		{
			CreateQValues();
		}

		// find current state

		State newState = GetCurrentState();

		// evaluate last action

		// add reward to QNode value

		float reward = EvaluateAction(m_CurrentState, newState);

		if (m_CurrentNode != QNode{}) m_CurrentNode.value += (reward * m_LearningRate);

		// should use random or Q value action?

		float randomValue = FMath::FRandRange(0.0f, 1.0f);

		ActionState action;

		if (randomValue < m_ExplorationRate)
		{
			// random action
			action = GenerateRandomAction();
		}
		else
		{
			// Q node action
			action = GetBestQNodeAction(newState);
		}

		// use computed action

		m_NPCRef->UseAction(action);

		// set current state to new state

		m_CurrentState = newState;

		for (const QNode& node : m_QValues)
		{
			if (node.currentState == newState && node.action == action)
			{
				m_CurrentNode = node;
				break;
			}
		}
	}
}


float UPlanningBrain::EvaluateAction(State first, State second)
{
	float totalWeight{ 0.0f };
	for (Quality quality : m_NPCRef->GetQualitiesFromMemory())
	{
		if (quality == Quality::Fearless)
		{
			// increase reward when health decreases
			if (first.healthPercentage < second.healthPercentage)
			{
				totalWeight++;
			}
			// decrease reward when health increases
			else if (first.healthPercentage > second.healthPercentage)
			{
				totalWeight--;
			}

			// otherwise no change in rewards
		}
		else if (quality == Quality::Coward)
		{
			if (first.healthPercentage < second.healthPercentage)
			{
				totalWeight++;
			}
			else if (first.healthPercentage > second.healthPercentage)
			{
				totalWeight--;
			}
		}
		else if (quality == Quality::MoralCompass)
		{
			TArray<AActor*> NPCs;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);
			int32 rewardAverage{ 0 };
			int32 numberOfAllies{ 0 };

			FVector2D selfLocation = FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y };
			for (AActor* npc : NPCs)
			{
				if (Cast<ANPC>(npc) != m_NPCRef)
				{
					if (Cast<ANPC>(npc)->m_Threat == m_NPCRef->m_Threat)
					{
						// get npc current state
						// if the current health is higher than the current state health, this NPC has likely healed them
						// if allies gained health increase reward
						FVector2D otherLocation = FVector2D{ npc->GetActorLocation().X, npc->GetActorLocation().Y };
						numberOfAllies++;
						if (FVector2D::Distance(otherLocation, selfLocation) <= KSENSORYRANGE)
						{
							if (Cast<ANPC>(npc)->CallGetCurrentState().healthPercentage > Cast<ANPC>(npc)->CallGetLastState().healthPercentage)
							{
								rewardAverage++;
							}
							// if allies lost health reduce reward
							else if (Cast<ANPC>(npc)->CallGetCurrentState().healthPercentage < Cast<ANPC>(npc)->CallGetLastState().healthPercentage)
							{
								rewardAverage--;
							}
						}
					}
				}
			}

			if (numberOfAllies != 0)
			{
				rewardAverage /= numberOfAllies;

				totalWeight += rewardAverage;
			}
		}
		else if (quality == Quality::Evil)
		{
			TArray<AActor*> NPCs;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);
			int32 rewardAverage{ 0 };
			int32 numberOfAllies{ 0 };

			FVector2D selfLocation = FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y };
			for (AActor* npc : NPCs)
			{
				if (Cast<ANPC>(npc) != m_NPCRef)
				{
					if (Cast<ANPC>(npc)->m_Threat == m_NPCRef->m_Threat)
					{
						// get npc current state
						// if the current health is higher than the current state health, this NPC has likely healed them
						// if allies lost health increase reward
						FVector2D otherLocation = FVector2D{ npc->GetActorLocation().X, npc->GetActorLocation().Y };
						numberOfAllies++;
						if (FVector2D::Distance(otherLocation, selfLocation) <= KSENSORYRANGE)
						{
							if (Cast<ANPC>(npc)->CallGetCurrentState().healthPercentage < Cast<ANPC>(npc)->CallGetLastState().healthPercentage)
							{
								rewardAverage++;
							}
							// if allies gained health reduce reward
							else if (Cast<ANPC>(npc)->CallGetCurrentState().healthPercentage > Cast<ANPC>(npc)->CallGetLastState().healthPercentage)
							{
								rewardAverage--;
							}
						}
					}
				}
			}

			if (numberOfAllies != 0)
			{
				rewardAverage /= numberOfAllies;

				totalWeight += rewardAverage;
			}
		}
		else if (quality == Quality::Violent)
		{
			// if damage was dealt increase reward
			if (m_NPCRef->GetDamageDealt() > 0.0f)
			{
				totalWeight++;
				m_NPCRef->SetDamageDealt(0.0f);
			}
		}
		else if (quality == Quality::Pacifist)
		{
			// if damage was dealt decrease reward
			if (m_NPCRef->GetDamageDealt() > 0.0f)
			{
				totalWeight--;
				m_NPCRef->SetDamageDealt(0.0f);
			}
		}
		else if (quality == Quality::Lazy)
		{
			// if action used was doing nothing and there were no allies or enemies are in range increase reward
			if (first.numberOfAlliesInRange <= 0 && first.numberOfEnemiesInRange <= 0)
			{
				if (m_CurrentNode.action == ActionState::DoingNothing)
				{
					totalWeight++;
				}
			}
		}
		else if (quality == Quality::Active)
		{
			// if action used was search and there were no allies or enemies are in range increase reward
			if (first.numberOfAlliesInRange <= 0 && first.numberOfEnemiesInRange <= 0)
			{
				if (m_CurrentNode.action == ActionState::Searching)
				{
					totalWeight++;
				}
			}
		}
	}

	return totalWeight / m_NPCRef->GetQualitiesFromMemory().Num(); // return the average weight
}

State UPlanningBrain::GetCurrentState()
{
	State currentState = State{};
	currentState.healthPercentage = static_cast<int32>(m_NPCRef->GetHealth() / m_NPCRef->GetMaxHealth());

	TArray<AActor*> NPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);

	int32 numberOfEnemiesInRange{ 0 }, numberOfAlliesInRange{ 0 };

	FVector2D selfLocation = FVector2D{ m_NPCRef->GetActorLocation().X, m_NPCRef->GetActorLocation().Y };

	for (AActor* npc : NPCs)
	{
		if (Cast<ANPC>(npc) != m_NPCRef)
		{
			FVector2D otherLocation = FVector2D{ npc->GetActorLocation().X, npc->GetActorLocation().Y };
			if (FVector2D::Distance(otherLocation, selfLocation) <= KSENSORYRANGE)
			{
				if (Cast<ANPC>(npc)->m_Threat == m_NPCRef->m_Threat)
				{
					numberOfEnemiesInRange++;
				}
				else {
					numberOfAlliesInRange++;
				}
			}
		}
	}

	currentState.numberOfAlliesInRange = numberOfAlliesInRange;
	currentState.numberOfEnemiesInRange = numberOfEnemiesInRange;

	return currentState;
}

void UPlanningBrain::GenerateStartingValues()
{
	CreateQValues();
	m_CurrentState = GetCurrentState();

	if (m_NPCRef->GetQualitiesFromMemory().Contains(Quality::Efficient) && m_LearningRate != 0.9f)
	{
		m_LearningRate = 0.9f;
	}
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

void UPlanningBrain::CreatePossibleStates()
{
	TArray<AActor*> NPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC::StaticClass(), NPCs);

	int32 numberOfEnemies{ 0 }, numberOfAllies{ 0 };

	for (AActor* npc : NPCs)
	{
		if (Cast<ANPC>(npc) != m_NPCRef)
		{
			if (Cast<ANPC>(npc)->IsThreat() != m_NPCRef->IsThreat())
			{
				numberOfEnemies++;
			}
			else 
			{
				numberOfAllies++;
			}
		}
	}

	m_NumberOfAllies = numberOfAllies;
	m_NumberOfEnemies = numberOfEnemies;

	if (numberOfAllies <= 0)
	{
		if (numberOfEnemies <= 0)
		{
			for (int k = 0; k < static_cast<int32>(m_NPCRef->GetHealth() / m_NPCRef->GetMaxHealth()); k++)
			{
				State newState = State{};
				newState.numberOfAlliesInRange = 0;
				newState.numberOfEnemiesInRange = 0;
				newState.healthPercentage = k;
				m_PossibleStates.Add(newState);
			}
		}
		else
		{
			for (int j = 0; j < numberOfEnemies; j++)
			{
				for (int k = 0; k < static_cast<int32>(m_NPCRef->GetHealth() / m_NPCRef->GetMaxHealth()); k++)
				{
					State newState = State{};
					newState.numberOfAlliesInRange = 0;
					newState.numberOfEnemiesInRange = j;
					newState.healthPercentage = k;
					m_PossibleStates.Add(newState);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < numberOfAllies; i++)
		{
			for (int j = 0; j < numberOfEnemies; j++)
			{
				for (int k = 0; k < static_cast<int32>(m_NPCRef->GetHealth() / m_NPCRef->GetMaxHealth()); k++)
				{
					State newState = State{};
					newState.numberOfAlliesInRange = i;
					newState.numberOfEnemiesInRange = j;
					newState.healthPercentage = k;
					m_PossibleStates.Add(newState);
				}
			}
		}
	}
}

void UPlanningBrain::CreateQValues()
{
	m_PossibleStates.Empty();
	CreatePossibleStates();
	for (State state : m_PossibleStates)
	{
		QNode newNode;
		newNode.currentState = state;
		newNode.action = ActionState::Attacking;
		newNode.value = 0.5f;
		if(!m_QValues.Contains(newNode)) m_QValues.Add(newNode);
		newNode.action = ActionState::Interacting;
		if (!m_QValues.Contains(newNode)) m_QValues.Add(newNode);
		newNode.action = ActionState::Searching;
		if (!m_QValues.Contains(newNode)) m_QValues.Add(newNode);
		newNode.action = ActionState::Following;
		if (!m_QValues.Contains(newNode)) m_QValues.Add(newNode);
		newNode.action = ActionState::RunningAway;
		if (!m_QValues.Contains(newNode)) m_QValues.Add(newNode);
		newNode.action = ActionState::UsingAbility;
		if (!m_QValues.Contains(newNode)) m_QValues.Add(newNode);
		newNode.action = ActionState::DoingNothing;
		if (!m_QValues.Contains(newNode)) m_QValues.Add(newNode);
	}
}

Path UPlanningBrain::FindAStarPath(ATile3D* startTile, ATile3D* endTile)
{
	if (startTile == nullptr || endTile == nullptr || startTile == endTile) {
		Path path;
		path.totalCost = -1;
		return path;
	}

	float heuristicCost{ 0 }; // going to use euclidean distance heuristic

	bool foundPath{ false };

	TArray<FNode> closedList; // checked tiles
	TArray<FNode> openList; // tiles to be checked
	
	FNode startNode = FNode(0, FVector::Distance(startTile->GetActorLocation(), endTile->GetActorLocation()), startTile, nullptr);

	openList.Add(startNode);

	while (foundPath == false && openList.Num() != 0) // while there are tiles to be checked and thus path has not been found
	{
		// find node in open list with lowest total cost
		FNode closestNode = openList[0];
		for (const FNode& node : openList)
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
		for (const FNode& node : closedList)
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
				for (const FNode& node : closedList)
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

ActionState UPlanningBrain::GenerateRandomAction()
{
	int32 randomInt = FMath::RandRange(0, KNUMBEROFPOSSIBLEACTIONS - 1);
	return static_cast<ActionState>(randomInt);
}

ActionState UPlanningBrain::GetBestQNodeAction(State state)
{
	QNode largestQNodeValueForCurrentState;
	largestQNodeValueForCurrentState.value = 0.0f;
	for (QNode node : m_QValues)
	{
		if (node.currentState == state)
		{
			if (node.value > largestQNodeValueForCurrentState.value)
			{
				largestQNodeValueForCurrentState = node;
			}
		}
	}

	return largestQNodeValueForCurrentState.action;
}

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