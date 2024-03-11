// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator.h"
#include "Math/UnrealMathUtility.h"

MapGenerator::MapGenerator()
{
	// randomize number of chunks
	m_NumXChunks = FMath::RandRange(3, 5);
	m_NumYChunks = FMath::RandRange(2, 5);

	for (int chunkY = 0; chunkY < m_NumYChunks; chunkY++)
	{
		FChunkRow newChunkRow;
		for (int chunkX = 0; chunkX < m_NumXChunks; chunkX++)
		{
			// for each new chunk, randomise the room type, ensure reception, break room & meeting room cannot have duplicates
			FChunk newChunk;
			newChunk.chunkNum = (chunkY * m_NumXChunks) + chunkX;
			// randomize bounds
			m_XBounds = FMath::RandRange(10, 20);
			m_YBounds = FMath::RandRange(10, 20);
			for (int y = 0; y < m_YBounds; y++)
			{
				FRow newRow;
				TArray<Tile> generatedMap;

				for (int x = 0; x < m_XBounds; x++)
				{
					Tile newTile;
					newTile.chunkNum = (chunkY * m_NumXChunks) + chunkX;
					int32 randomTile = FMath::RandRange(0, 2);

					if (randomTile == 0)
					{
						newTile.property = TileProp::FloorTile;
					}
					else if (randomTile == 1)
					{
						newTile.property = TileProp::WallTile;
					}
					else
					{
						newTile.property = TileProp::NoTile;
					}

					generatedMap.Add(newTile);
				}
				newRow.index = generatedMap;

				newChunk.chunkMap.Add(newRow);
			}
			newChunk.xBound = m_XBounds;
			newChunk.yBound = m_YBounds;

			newChunkRow.chunks.Add(newChunk);
		}
		m_Chunks.Add(newChunkRow);
	}
	for (int y = 0; y < m_NumYChunks; y++)
	{
		for (int x = 0; x < m_NumXChunks; x++)
		{
			// create type of room
			m_Chunks[y].chunks[x].chunkType = GenerateRandomRoomType();
		}
	}

	

	

}

MapGenerator::~MapGenerator()
{
}

Room MapGenerator::GenerateRandomRoomType()
{
	// check chunks for reception, meeting room & break room, ensure no duplicates
	bool hasReception{ false }, hasMeetingRoom{ false }, hasBreakRoom{ false }, hasOffice{ false }, hasResearchRoom{ false },
		hasCell{ false }, allRoomsSpawned{ false };

	for (int y = 0; y < m_Chunks.Num(); y++)
	{
		for (int x = 0; x < m_Chunks[y].chunks.Num(); x++)
		{
			if (m_Chunks[y].chunks[x].chunkType == Room::Reception)
			{
				hasReception = true;
			}
			else if (m_Chunks[y].chunks[x].chunkType == Room::MeetingRoom)
			{
				hasMeetingRoom = true;
			}
			else if (m_Chunks[y].chunks[x].chunkType == Room::BreakRoom)
			{
				hasBreakRoom = true;
			}
			else if (m_Chunks[y].chunks[x].chunkType == Room::Office)
			{
				hasOffice = true;
			}
			else if (m_Chunks[y].chunks[x].chunkType == Room::ResearchRoom)
			{
				hasResearchRoom = true;
			}
			else if (m_Chunks[y].chunks[x].chunkType == Room::Cell)
			{
				hasCell = true;
			}
		}
	}
	// if all rooms exist, go with random room from duplicate rooms (office, research room, cell etc.)
	allRoomsSpawned = hasReception && hasMeetingRoom && hasBreakRoom && hasOffice && hasResearchRoom && hasCell;

	bool validType{ false };

	Room roomType;

	while (!validType)
	{
		int32 randomRoomType = FMath::RandRange(0, KNUMOFROOMS - 2);

		switch (randomRoomType)
		{
		case 0:
			if (!hasReception)
			{
				roomType = Room::Reception;
				validType = true;
			}
			break;
		case 1:
			if (!hasMeetingRoom)
			{
				roomType = Room::MeetingRoom;
				validType = true;
			}
			break;
		case 2:
			if (!hasBreakRoom)
			{
				roomType = Room::BreakRoom;
				validType = true;
			}
			break;
		case 3:
			if (!hasResearchRoom || allRoomsSpawned)
			{
				roomType = Room::ResearchRoom;
				validType = true;
			}
			break;
		case 4:
			if (!hasCell || allRoomsSpawned)
			{
				roomType = Room::Cell;
				validType = true;
			}
			break;
		case 5:
			if (!hasOffice || allRoomsSpawned)
			{
				roomType = Room::Office;
				validType = true;
			}
			break;
		default:
			roomType = Room::Hallway;
			validType = true;
			break;
		}
	}

	return roomType;
}

float MapGenerator::GetMapTraversability(const TArray<FRow>& tileMap)
{
	float totalFloorTiles{ 0.0f };
	float totalTraversableTiles{ 0.0f };
	float averageTraversability{ 0.0f };

	// for each floor tile, find all floor tiles excluding self and see if they can be pathed to, if so add to fitness

	for (int y = 0; y < m_YBounds; y++)
	{
		for (int x = 0; x < m_XBounds; x++)
		{
			if (tileMap[y].index[x].property == TileProp::FloorTile)
			{
				totalFloorTiles++;
				for (int i = 0; i < m_YBounds; i++)
				{
					for (int j = 0; j < m_XBounds; j++)
					{
						if (tileMap[i].index[j].property == TileProp::FloorTile)
						{
							FIndex first, second;
							first.x = x;
							first.y = y;
							second.x = j;
							second.y = i;
							if (CanTraverseBreadthFirstSearch(first, second, tileMap))
							{
								totalTraversableTiles++;
							}
						}
					}
				}
			}
		}
	}

	return (totalTraversableTiles / (totalFloorTiles * totalFloorTiles)); // where 1.0f is a fully traversable map and 0.0f is a fully non traversable map
}

bool MapGenerator::CanTraverseBreadthFirstSearch(FIndex first, FIndex second, const TArray<FRow>& tileMap)
{
	TArray<FIndex> openList;
	TArray<FIndex> closedList;

	openList.Add(first);

	// breadth first search
	while (openList.Num() > 0)
	{
		FIndex currentItem;
		currentItem.x = openList[0].x;
		currentItem.y = openList[0].y;
		openList.RemoveAt(0);

		if (currentItem == second) {
			return true;
		}

		TArray<FIndex> adjacentTiles = GetNeighbours(currentItem);

		for (FIndex tile : adjacentTiles)
		{
			if (tile.x >= 0 && tile.x < m_XBounds && tile.y >= 0 && tile.y < m_YBounds)
			{
				if (tileMap[tile.y].index[tile.x].property == TileProp::FloorTile && !openList.Contains(tile) && !closedList.Contains(tile)) // if neighbour isnt in open or closed list and is a floor tile, traverse
				{
					openList.Add(tile);
				}
			}
		}

		closedList.Add(currentItem);

	}


	return false;
}

bool MapGenerator::CanTraverseBestFirstSearch(FIndex first, FIndex second, const TArray<FRow>& tileMap)
{
	TArray<FRow> newMap = tileMap;

	for (int i = 0; i < newMap.Num(); i++)
	{
		newMap[i].EmptyLists();
	}

	newMap[first.y].index[first.x].list = ListType::Open;
	bool areItemsInOpenList{ true };
	while (areItemsInOpenList == true)
	{
		// use the index with the lowest heuristic distance, in this case, manhattan heuristic will be used
		// the lowest number when the x difference and y difference between this index and the target index and are summed
		FIndex currentItem;
		currentItem.x = -1;
		currentItem.y = -1;
		for (int i = 0; i < newMap.Num(); i++)
		{
			for (int j = 0; j < newMap[i].index.Num(); j++)
			{
				if (newMap[i].index[j].list == ListType::Open)
				{
					if (currentItem.x < 0 || currentItem.y < 0)
					{
						currentItem.x = j;
						currentItem.y = i;
					}

					int32 newManhattanHeuristic = (abs(j - second.x)) + (abs(i - second.y));
					int32 oldManhattanHeuristic = (abs(currentItem.x - second.x)) + (abs(currentItem.y - second.y));
					if (newManhattanHeuristic < oldManhattanHeuristic)
					{
						currentItem.x = j;
						currentItem.y = i;
					}
				}
			}
		}

		if (currentItem == second) {
			return true;
		}

		TArray<FIndex> adjacentTiles = GetNeighbours(currentItem);
		
		for (FIndex tile : adjacentTiles)
		{
			// only search directly adjacent tiles (above, below, left or right)
			if (tile.directlyAdjacent)
			{
				if (tile.x >= 0 && tile.x < m_XBounds && tile.y >= 0 && tile.y < m_YBounds)
				{
					if (newMap[tile.y].index[tile.x].list == ListType::None && newMap[tile.y].index[tile.x].property == TileProp::FloorTile)
					{
						newMap[tile.y].index[tile.x].list = ListType::Open;
					}
				}
			}
		}

		newMap[currentItem.y].index[currentItem.x].list = ListType::Closed;


		areItemsInOpenList = false;
		for (int i = 0; i < newMap.Num(); i++)
		{
			for (int j = 0; j < newMap[i].index.Num(); j++)
			{
				if (newMap[i].index[j].list == ListType::Open)
				{
					areItemsInOpenList = true;
				}
			}
		}
	}

	return false;
}

bool MapGenerator::AreAllChunksConnected()
{
	// get a random tile, doesn't matter which

	FIndex randomTile;

	for (int i = 0; i < m_YBounds; i++)
	{
		for (int j = 0; j < m_XBounds; j++)
		{
			if (m_Map[i].index[j].property == TileProp::FloorTile)
			{
				randomTile.x = j;
				randomTile.y = i;
				break;
			}
		}
		if (randomTile.x >= 0) break;
	}
	if (randomTile.x < 0) return false;

	// use this tile to check against all other tiles
	for (int i = 0; i < m_YBounds; i++)
	{
		for (int j = 0; j < m_XBounds; j++)
		{
			if (m_Map[i].index[j].property == TileProp::FloorTile)
			{
				FIndex first, second;
				first.x = randomTile.x;
				first.y = randomTile.y;
				second.x = j;
				second.y = i;
				if (!CanTraverseBestFirstSearch(first, second, m_Map))
				{
					return false;
				}
			}
		}
	}

	return true;
}

TArray<FIndex> MapGenerator::GetNeighbours(FIndex passedIndex)
{
	TArray<FIndex> neighbours;
	FIndex newNeighbour;

	// top

	newNeighbour.x = passedIndex.x;
	newNeighbour.y = passedIndex.y - 1;
	newNeighbour.directlyAdjacent = true;
	neighbours.Add(newNeighbour);

	// top right

	newNeighbour.x = passedIndex.x + 1;
	newNeighbour.y = passedIndex.y - 1;
	newNeighbour.directlyAdjacent = false;
	neighbours.Add(newNeighbour);

	// top left

	newNeighbour.x = passedIndex.x - 1;
	newNeighbour.y = passedIndex.y - 1;
	newNeighbour.directlyAdjacent = false;
	neighbours.Add(newNeighbour);

	// left

	newNeighbour.x = passedIndex.x - 1;
	newNeighbour.y = passedIndex.y;
	newNeighbour.directlyAdjacent = true;
	neighbours.Add(newNeighbour);

	// right

	newNeighbour.x = passedIndex.x + 1;
	newNeighbour.y = passedIndex.y;
	newNeighbour.directlyAdjacent = true;
	neighbours.Add(newNeighbour);

	// bottom

	newNeighbour.x = passedIndex.x;
	newNeighbour.y = passedIndex.y + 1;
	newNeighbour.directlyAdjacent = true;
	neighbours.Add(newNeighbour);

	// bottom right

	newNeighbour.x = passedIndex.x + 1;
	newNeighbour.y = passedIndex.y + 1;
	newNeighbour.directlyAdjacent = false;
	neighbours.Add(newNeighbour);

	// bottom left

	newNeighbour.x = passedIndex.x - 1;
	newNeighbour.y = passedIndex.y + 1;
	newNeighbour.directlyAdjacent = false;
	neighbours.Add(newNeighbour);

	return neighbours;
}

void MapGenerator::RefineMap()
{
	bool allChunksComplete{ true };

	for (int y = 0; y < m_NumYChunks; y++)
	{
		for (int x = 0; x < m_NumXChunks; x++)
		{
			if (!m_Chunks[y].chunks[x].completed)
			{
				allChunksComplete = false;
				m_Chunks[y].chunks[x] = RefineChunk(m_Chunks[y].chunks[x]);
			}
		}
	}

	if (allChunksComplete)
	{
		// concatenate all chunks into m_Map, best first construct paths, refine and then mark map as completed
		m_Map = ConcatenateChunksIntoMap();
		
		// construct paths between chunks by creating random pairs of chunks and running a path creation algorithm
		bool pathsConnected{ false };
		do {
			int32 firstRandomXChunk = FMath::RandRange(0, m_NumXChunks - 1);
			int32 firstRandomYChunk = FMath::RandRange(0, m_NumYChunks - 1);
			int32 secondRandomXChunk = firstRandomXChunk + FMath::RandRange(-1, 1);
			int32 secondRandomYChunk = firstRandomYChunk + FMath::RandRange(-1, 1);

			if (firstRandomXChunk == secondRandomXChunk && firstRandomYChunk == secondRandomYChunk)
			{
				// do nothing
			}
			else
			{
				if (secondRandomXChunk >= 0 && secondRandomXChunk < m_NumXChunks && secondRandomYChunk >= 0 && secondRandomYChunk < m_NumYChunks)
				{
					m_Map = ConnectChunks(m_Map, m_Chunks[firstRandomYChunk].chunks[firstRandomXChunk].chunkNum, m_Chunks[secondRandomYChunk].chunks[secondRandomXChunk].chunkNum);

					
					m_Chunks[firstRandomYChunk].chunks[firstRandomXChunk].isPaired = true;
					m_Chunks[secondRandomYChunk].chunks[secondRandomXChunk].isPaired = true;
				}
			}

			// get random indices from every chunk and check if they can each traverse between each other
			pathsConnected = AreAllChunksConnected();
		} while (!pathsConnected);

		// final touches
		m_Map = FinalRefinement(m_Map, m_XBounds, m_YBounds);
		m_Map = FinalRefinement(m_Map, m_XBounds, m_YBounds, true); // run again to get rid of artefacts
		m_MapGenerated = true;

	}
}

FChunk MapGenerator::RefineChunk(const FChunk& tileMap)
{
	//
	// chunk grammar based method
	//


	FChunk generatedMap = tileMap;
	bool changedTile{ false };

	for (int y = 0; y < tileMap.yBound; y++)
	{
		for (int x = 0; x < tileMap.xBound; x++)
		{
			// find neighbours, if none are floor tiles, replace with a none tile, also if this tile is on the edge of the map, replace with a none or wall tile
			// if there are less than 3 floor tiles surrounding this floor tile, replace with either a wall or none tile
			if (generatedMap.chunkMap[y].index[x].property == TileProp::FloorTile)
			{
				int32 numOfFloorTiles{ 0 };
				bool adjacentToFloor{ false };
				bool onEdge{ false };
				FIndex floorIndex;
				floorIndex.x = x;
				floorIndex.y = y;
				TArray<FIndex> adjacentTiles = GetNeighbours(floorIndex);

				for (FIndex tile : adjacentTiles)
				{
					if (tile.x >= 0 && tile.x < tileMap.xBound && tile.y >= 0 && tile.y < tileMap.yBound)
					{
						if (generatedMap.chunkMap[tile.y].index[tile.x].property == TileProp::FloorTile)
						{
							adjacentToFloor = true;
							numOfFloorTiles++;
						}
					}
					else
					{
						onEdge = true;
					}
				}

				if (!adjacentToFloor)
				{
					generatedMap.chunkMap[y].index[x].property = TileProp::NoTile;
					changedTile = true;
				}
				if (onEdge)
				{
					int32 randNum = FMath::RandRange(0, 1);
					if (randNum == 0) generatedMap.chunkMap[y].index[x].property = TileProp::NoTile;
					else if (randNum == 1) generatedMap.chunkMap[y].index[x].property = TileProp::WallTile;
					changedTile = true;
				}
				if (numOfFloorTiles < 3)
				{
					int32 randNum = FMath::RandRange(0, 1);
					if (randNum == 0) generatedMap.chunkMap[y].index[x].property = TileProp::NoTile;
					else if (randNum == 1) generatedMap.chunkMap[y].index[x].property = TileProp::WallTile;
					changedTile = true;
				}
			}
			// find neighbours, if none are floor tiles, replace with a none tile
			// if there are six or more floor tiles surrounding this tile, replace with a floor tile
			else if (generatedMap.chunkMap[y].index[x].property == TileProp::WallTile)
			{
				int32 numOfFloorTiles{ 0 };
				bool adjacentToFloor{ false };
				FIndex floorIndex;
				floorIndex.x = x;
				floorIndex.y = y;
				TArray<FIndex> adjacentTiles = GetNeighbours(floorIndex);

				for (FIndex tile : adjacentTiles)
				{
					if (tile.x >= 0 && tile.x < tileMap.xBound && tile.y >= 0 && tile.y < tileMap.yBound)
					{
						if (generatedMap.chunkMap[tile.y].index[tile.x].property == TileProp::FloorTile)
						{
							adjacentToFloor = true;
							numOfFloorTiles++;
						}
					}
				}

				if (!adjacentToFloor)
				{
					generatedMap.chunkMap[y].index[x].property = TileProp::NoTile;
					changedTile = true;
				}
				if (numOfFloorTiles >= 6)
				{
					generatedMap.chunkMap[y].index[x].property = TileProp::FloorTile;
					changedTile = true;
				}
			}
			// find neighbours, if there is a floor tile, replace with either a wall tile or a floor tile
			else 
			{
				bool adjacentToFloor{ false };
				FIndex floorIndex;
				floorIndex.x = x;
				floorIndex.y = y;
				TArray<FIndex> adjacentTiles = GetNeighbours(floorIndex);

				for (FIndex tile : adjacentTiles)
				{
					if (tile.x >= 0 && tile.x < tileMap.xBound && tile.y >= 0 && tile.y < tileMap.yBound)
					{
						if (generatedMap.chunkMap[tile.y].index[tile.x].property == TileProp::FloorTile)
						{
							adjacentToFloor = true;
						}
					}
				}

				if (adjacentToFloor)
				{
					int32 randNum = FMath::RandRange(0, 1);
					if(randNum == 0) generatedMap.chunkMap[y].index[x].property = TileProp::FloorTile;
					else if (randNum == 1) generatedMap.chunkMap[y].index[x].property = TileProp::WallTile;
					changedTile = true;
				}
			}
		}
	}

	if (!changedTile)
	{
		generatedMap.chunkMap = ConstructPath(generatedMap.chunkMap, generatedMap.xBound, generatedMap.yBound); // create paths between all floor tiles
		generatedMap.chunkMap = FinalRefinement(generatedMap.chunkMap, generatedMap.xBound, generatedMap.yBound); // conduct final refinement
		generatedMap.completed = true;
		return generatedMap;
	}
	return generatedMap;
}

TArray<FRow> MapGenerator::FinalRefinement(const TArray<FRow>& tileMap, int32 xBounds, int32 yBounds, bool finalPass)
{
	TArray<FRow> newMap = tileMap;
	// after map is done, replace all wall tiles not adjacent to a none tile with floor tiles
	for (int y = 0; y < yBounds; y++)
	{
		for (int x = 0; x < xBounds; x++)
		{
			if (newMap[y].index[x].property == TileProp::WallTile)
			{
				bool adjacentToNone{ false };
				FIndex floorIndex;
				floorIndex.x = x;
				floorIndex.y = y;
				TArray<FIndex> adjacentTiles = GetNeighbours(floorIndex);

				for (FIndex tile : adjacentTiles)
				{
					if (tile.x >= 0 && tile.x < xBounds && tile.y >= 0 && tile.y < yBounds)
					{
						if (newMap[tile.y].index[tile.x].property == TileProp::NoTile)
						{
							adjacentToNone = true;
						}
					}
				}

				if (!adjacentToNone)
				{
					newMap[y].index[x].property = TileProp::FloorTile;
				}
			}
		}
	}
	// after walls are refined, refine floors by replacing edge floor tiles with wall tiles
	for (int y = 0; y < yBounds; y++)
	{
		for (int x = 0; x < xBounds; x++)
		{
			if (newMap[y].index[x].property == TileProp::FloorTile)
			{
				bool isEdge{ false };
				FIndex floorIndex;
				floorIndex.x = x;
				floorIndex.y = y;
				TArray<FIndex> adjacentTiles = GetNeighbours(floorIndex);

				for (FIndex tile : adjacentTiles)
				{
					if (tile.x >= 0 && tile.x < xBounds && tile.y >= 0 && tile.y < yBounds)
					{
						// do nothing
					}
					else
					{
						isEdge = true;
					}
				}

				if (isEdge && finalPass)
				{
					newMap[y].index[x].property = TileProp::WallTile;
				}
			}
			else if (newMap[y].index[x].property == TileProp::NoTile) // all none tiles adjacent to floor tiles become wall tiles
			{
				bool adjacentToFloor{ false };
				FIndex floorIndex;
				floorIndex.x = x;
				floorIndex.y = y;
				TArray<FIndex> adjacentTiles = GetNeighbours(floorIndex);

				for (FIndex tile : adjacentTiles)
				{
					if (tile.x >= 0 && tile.x < xBounds && tile.y >= 0 && tile.y < yBounds)
					{
						if (newMap[tile.y].index[tile.x].property == TileProp::FloorTile)
						{
							adjacentToFloor = true;
						}
					}
				}

				if (adjacentToFloor)
				{
					newMap[y].index[x].property = TileProp::WallTile;
				}
			}
		}
	}
	return newMap;
}

TArray<FRow> MapGenerator::ConstructPath(const TArray<FRow>& tileMap, int32 xBounds, int32 yBounds)
{
	TArray<FRow> newMap = tileMap;
	int32 randomYIndex;
	int32 randomXIndex;
	// find random floor tile
	do {
		randomYIndex = FMath::RandRange(0, yBounds - 1);
		randomXIndex = FMath::RandRange(0, xBounds - 1);
	} while (tileMap[randomYIndex].index[randomXIndex].property != TileProp::FloorTile);

	for (int i = 0; i < yBounds; i++)
	{
		for (int j = 0; j < xBounds; j++)
		{
			// find the path between every other floor tile, in cases where no such path exists, the algorithm constructs new paths
			if (tileMap[i].index[j].property == TileProp::FloorTile)
			{
				FIndex first, second;
				first.x = randomXIndex;
				first.y = randomYIndex;
				second.x = j;
				second.y = i;
				newMap = BestFirstPathConstruction(first, second, newMap, xBounds, yBounds);
			}
		}
	}
		
	

	return newMap;
}


TArray<FRow> MapGenerator::BestFirstPathConstruction(FIndex start, FIndex target, const TArray<FRow>& tileMap, int32 xBounds, int32 yBounds)
{
	// first of all find if a path exists
	bool pathExists{ false };

	TArray<FRow> newMap = tileMap;

	for (int i = 0; i < newMap.Num(); i++)
	{
		newMap[i].EmptyLists();
	}

	newMap[start.y].index[start.x].list = ListType::Open;

	while (pathExists == false)
	{
		// use the index with the lowest heuristic distance, in this case, manhattan heuristic will be used
		// the lowest number when the x difference and y difference between this index and the target index and are summed
		FIndex currentItem;
		currentItem.x = -1;
		currentItem.y = -1;
		for (int i = 0; i < newMap.Num(); i++)
		{
			for (int j = 0; j < newMap[i].index.Num(); j++)
			{
				if (newMap[i].index[j].list == ListType::Open)
				{
					if (currentItem.x < 0 || currentItem.y < 0)
					{
						currentItem.x = j;
						currentItem.y = i;
					}

					int32 newManhattanHeuristic = (abs(j - target.x)) + (abs(i - target.y));
					int32 oldManhattanHeuristic = (abs(currentItem.x - target.x)) + (abs(currentItem.y - target.y));
					if (newManhattanHeuristic < oldManhattanHeuristic)
					{
						currentItem.x = j;
						currentItem.y = i;
					}
				}
			}
		}

		if (currentItem == target) {
			pathExists = true;
		}

		TArray<FIndex> adjacentTiles = GetNeighbours(currentItem);

		for (FIndex tile : adjacentTiles)
		{
			// only search directly adjacent tiles (above, below, left or right)
			if (tile.directlyAdjacent)
			{
				if (tile.x >= 0 && tile.x < xBounds && tile.y >= 0 && tile.y < yBounds)
				{
					if (newMap[tile.y].index[tile.x].list == ListType::None) // if neighbour isnt in open or closed list, traverse
					{
						newMap[tile.y].index[tile.x].list = ListType::Open;
					}
				}
			}
		}

		newMap[currentItem.y].index[currentItem.x].list = ListType::Closed;
	}

	// convert closed list to floor tiles
	for (int i = 0; i < newMap.Num(); i++)
	{
		for (int j = 0; j < newMap[i].index.Num(); j++)
		{
			if (newMap[i].index[j].list == ListType::Closed)
			{
				newMap[i].index[j].property = TileProp::FloorTile;
			}
		}
	}

	return newMap;
	//}

	//if (pathExists)
	//{
	//	return tileMap;
	//}
	//else
	//{
	//	TArray<FRow> newMap = tileMap;

	//	// then from the closed list, find the index that was closest to the target and construct a best first path to the target
	//	FIndex currentItem = closedList[0];
	//	for (int i = 0; i < closedList.Num(); i++)
	//	{
	//		int32 newManhattanHeuristic = (abs(closedList[i].x - target.x)) + (abs(closedList[i].y - target.y));
	//		int32 oldManhattanHeuristic = (abs(currentItem.x - target.x)) + (abs(currentItem.y - target.y));
	//		if (newManhattanHeuristic < oldManhattanHeuristic)
	//		{
	//			currentItem = closedList[i];
	//		}
	//	}

	//	closedList.Empty();
	//	bool endReached{ false };
	//	int32 count{ 0 };
	//	while (!endReached)
	//	{
	//		// find neighbours of closest item, then use the closest one as the new closest item, converting it to a floor tile if necessary
	//		if (newMap[currentItem.y].index[currentItem.x].property != TileProp::FloorTile)
	//		{
	//			newMap[currentItem.y].index[currentItem.x].property = TileProp::FloorTile;
	//		}

	//		TArray<FIndex> adjacentTiles = GetNeighbours(currentItem);
	//		closedList.Add(currentItem); // add to new closed list
	//		currentItem = adjacentTiles[0];

	//		for (int i = 0; i < adjacentTiles.Num(); i++)
	//		{
	//			if (adjacentTiles[i].directlyAdjacent)
	//			{
	//				if (adjacentTiles[i] == target) {
	//					endReached = true;
	//					return newMap;
	//				}
	//				int32 newManhattanHeuristic = (abs(adjacentTiles[i].x - target.x)) + (abs(adjacentTiles[i].y - target.y));
	//				int32 oldManhattanHeuristic = (abs(currentItem.x - target.x)) + (abs(currentItem.y - target.y));
	//				if (newManhattanHeuristic < oldManhattanHeuristic)
	//				{
	//					currentItem = adjacentTiles[i];
	//				}
	//			}
	//		}
	//		count++;
	//	}
	//	return newMap;

	//}

	
}

TArray<FRow> MapGenerator::ConnectChunks(const TArray<FRow>& entireMap, const int32& firstChunkNum, const int32& secondChunkNum)
{
	// find any random tile from each chunk

	FIndex firstChunkTile, secondChunkTile;

	bool isValid{ false };
	while (!isValid)
	{
		firstChunkTile.x = FMath::RandRange(0, m_XBounds - 1);
		firstChunkTile.y = FMath::RandRange(0, m_YBounds - 1);

		if (entireMap[firstChunkTile.y].index[firstChunkTile.x].property == TileProp::FloorTile &&
			entireMap[firstChunkTile.y].index[firstChunkTile.x].chunkNum == firstChunkNum)
		{
			isValid = true;
		}

	}
	isValid = false;
	while (!isValid)
	{
		secondChunkTile.x = FMath::RandRange(0, m_XBounds - 1);
		secondChunkTile.y = FMath::RandRange(0, m_YBounds - 1);

		if (entireMap[secondChunkTile.y].index[secondChunkTile.x].property == TileProp::FloorTile &&
			entireMap[secondChunkTile.y].index[secondChunkTile.x].chunkNum == secondChunkNum)
		{
			isValid = true;
		}

	}


	// finally construct a new path between these two tiles using best first path construction
	TArray<FRow> newMap = entireMap;

	newMap = BestFirstPathConstruction(firstChunkTile, secondChunkTile, entireMap, m_XBounds, m_YBounds);

	return newMap;
}

TArray<FRow> MapGenerator::ConcatenateChunksIntoMap()
{
	TArray<FRow> newMap;

	// find the biggest bounds for y and x
	int32 largestYBounds{ 0 };
	int32 largestXBounds{ 0 };
	for (int chunkY = 0; chunkY < m_NumYChunks; chunkY++)
	{
		for (int chunkX = 0; chunkX < m_NumXChunks; chunkX++)
		{
			if (m_Chunks[chunkY].chunks[chunkX].yBound > largestYBounds)
			{
				int32 newLargestYBounds = m_Chunks[chunkY].chunks[chunkX].yBound;
				if (newLargestYBounds > largestYBounds) largestYBounds = newLargestYBounds;
			}
			if (m_Chunks[chunkY].chunks[chunkX].xBound > largestXBounds)
			{
				int32 newLargestXBounds = m_Chunks[chunkY].chunks[chunkX].xBound;
				if (newLargestXBounds > largestXBounds) largestXBounds = newLargestXBounds;
			}
		}
	}

	m_XBounds = (largestXBounds) * m_NumXChunks;
	m_YBounds = (largestYBounds) * m_NumYChunks;

	// go through each row of chunks, concatenate each row and add to map
	for (int chunkY = 0; chunkY < m_NumYChunks; chunkY++)
	{
		for (int y = 0; y < largestYBounds; y++)
		{
			FRow newRow;
			for (int chunkX = 0; chunkX < m_NumXChunks; chunkX++)
			{
				for (int x = 0; x < largestXBounds; x++)
				{
					Tile newTile;
					if (y < m_Chunks[chunkY].chunks[chunkX].yBound)
					{
						if (x < m_Chunks[chunkY].chunks[chunkX].xBound)
						{
							newTile.property = m_Chunks[chunkY].chunks[chunkX].chunkMap[y].index[x].property;
							newTile.roomType = m_Chunks[chunkY].chunks[chunkX].chunkType;
							newTile.chunkNum = m_Chunks[chunkY].chunks[chunkX].chunkNum;
						}
						else
						{
							newTile.property = TileProp::NoTile;
							newTile.roomType = Room::Hallway;
							newTile.chunkNum = m_Chunks[chunkY].chunks[chunkX].chunkNum;
						}
					}
					else
					{
						newTile.property = TileProp::NoTile;
						newTile.roomType = Room::Hallway;
						newTile.chunkNum = m_Chunks[chunkY].chunks[chunkX].chunkNum;
					}
					newRow.index.Add(newTile);
				}

			}
			newMap.Add(newRow);
		}
	}


	return newMap;
}