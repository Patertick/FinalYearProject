// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator.h"
#include "Math/UnrealMathUtility.h"

MapGenerator::MapGenerator()
{
	// randomize number of chunks
	m_NumXChunks = FMath::RandRange(2, 5);
	m_NumYChunks = FMath::RandRange(2, 5);

	for (int chunkY = 0; chunkY < m_NumYChunks; chunkY++)
	{
		FChunkRow newChunkRow;
		for (int chunkX = 0; chunkX < m_NumXChunks; chunkX++)
		{
			FChunk newChunk;
			// randomize bounds
			m_XBounds = FMath::RandRange(10, 20);
			m_YBounds = FMath::RandRange(10, 20);
			for (int y = 0; y < m_YBounds; y++)
			{
				FRow newRow;
				TArray<TEnumAsByte<Tile>> generatedMap;

				for (int x = 0; x < m_XBounds; x++)
				{
					int32 randomTile = FMath::RandRange(0, 2);

					if (randomTile == 0)
					{
						generatedMap.Add(Tile::FloorTile);
					}
					else if (randomTile == 1)
					{
						generatedMap.Add(Tile::WallTile);
					}
					else
					{
						generatedMap.Add(Tile::NoTile);
					}
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

	

}

MapGenerator::~MapGenerator()
{
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
			if (tileMap[y].index[x] == Tile::FloorTile)
			{
				totalFloorTiles++;
				for (int i = 0; i < m_YBounds; i++)
				{
					for (int j = 0; j < m_XBounds; j++)
					{
						if (tileMap[i].index[j] == Tile::FloorTile)
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
				if (tileMap[tile.y].index[tile.x] == Tile::FloorTile && !openList.Contains(tile) && !closedList.Contains(tile)) // if neighbour isnt in open or closed list and is a floor tile, traverse
				{
					openList.Add(tile);
				}
			}
		}

		closedList.Add(currentItem);

	}


	return false;
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
		m_Map = ConstructPath(m_Map);
		m_Map = FinalRefinement(m_Map, m_XBounds, m_YBounds);
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
			if (generatedMap.chunkMap[y].index[x] == Tile::FloorTile)
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
						if (generatedMap.chunkMap[tile.y].index[tile.x] == Tile::FloorTile)
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
					generatedMap.chunkMap[y].index[x] = Tile::NoTile;
					changedTile = true;
				}
				if (onEdge)
				{
					int32 randNum = FMath::RandRange(0, 1);
					if (randNum == 0) generatedMap.chunkMap[y].index[x] = Tile::NoTile;
					else if (randNum == 1) generatedMap.chunkMap[y].index[x] = Tile::WallTile;
					changedTile = true;
				}
				if (numOfFloorTiles < 3)
				{
					int32 randNum = FMath::RandRange(0, 1);
					if (randNum == 0) generatedMap.chunkMap[y].index[x] = Tile::NoTile;
					else if (randNum == 1) generatedMap.chunkMap[y].index[x] = Tile::WallTile;
					changedTile = true;
				}
			}
			// find neighbours, if none are floor tiles, replace with a none tile
			// if there are six or more floor tiles surrounding this tile, replace with a floor tile
			else if (generatedMap.chunkMap[y].index[x] == Tile::WallTile)
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
						if (generatedMap.chunkMap[tile.y].index[tile.x] == Tile::FloorTile)
						{
							adjacentToFloor = true;
							numOfFloorTiles++;
						}
					}
				}

				if (!adjacentToFloor)
				{
					generatedMap.chunkMap[y].index[x] = Tile::NoTile;
					changedTile = true;
				}
				if (numOfFloorTiles >= 6)
				{
					generatedMap.chunkMap[y].index[x] = Tile::FloorTile;
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
						if (generatedMap.chunkMap[tile.y].index[tile.x] == Tile::FloorTile)
						{
							adjacentToFloor = true;
						}
					}
				}

				if (adjacentToFloor)
				{
					int32 randNum = FMath::RandRange(0, 1);
					if(randNum == 0) generatedMap.chunkMap[y].index[x] = Tile::FloorTile;
					else if (randNum == 1) generatedMap.chunkMap[y].index[x] = Tile::WallTile;
					changedTile = true;
				}
			}
		}
	}

	if (!changedTile)
	{
		//generatedMap = ConstructPath(generatedMap); // create paths between all floor tiles
		generatedMap.chunkMap = FinalRefinement(generatedMap.chunkMap, generatedMap.xBound, generatedMap.yBound); // conduct final refinement
		generatedMap.completed = true;
		return generatedMap;
	}
	return generatedMap;
}

TArray<FRow> MapGenerator::FinalRefinement(const TArray<FRow>& tileMap, int32 xBounds, int32 yBounds)
{
	TArray<FRow> newMap = tileMap;
	// after map is done, replace all wall tiles not adjacent to a none tile with floor tiles
	for (int y = 0; y < yBounds; y++)
	{
		for (int x = 0; x < xBounds; x++)
		{
			if (newMap[y].index[x] == Tile::WallTile)
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
						if (newMap[tile.y].index[tile.x] == Tile::NoTile)
						{
							adjacentToNone = true;
						}
					}
				}

				if (!adjacentToNone)
				{
					newMap[y].index[x] = Tile::FloorTile;
				}
			}
		}
	}
	// after walls are refined, refine floors by replacing edge floor tiles with wall tiles
	for (int y = 0; y < yBounds; y++)
	{
		for (int x = 0; x < xBounds; x++)
		{
			if (newMap[y].index[x] == Tile::FloorTile)
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

				if (isEdge)
				{
					newMap[y].index[x] = Tile::WallTile;
				}
			}
			else if (newMap[y].index[x] == Tile::NoTile) // all none tiles adjacent to floor tiles become wall tiles
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
						if (newMap[tile.y].index[tile.x] == Tile::FloorTile)
						{
							adjacentToFloor = true;
						}
					}
				}

				if (adjacentToFloor)
				{
					newMap[y].index[x] = Tile::WallTile;
				}
			}
		}
	}
	return newMap;
}

TArray<FRow> MapGenerator::ConstructPath(const TArray<FRow>& tileMap)
{
	TArray<FRow> newMap = tileMap;
	// for each floor tile
	for (int y = 0; y < m_YBounds; y++)
	{
		for (int x = 0; x < m_XBounds; x++)
		{
			if (tileMap[y].index[x] == Tile::FloorTile)
			{
				for (int i = 0; i < m_YBounds; i++)
				{
					for (int j = 0; j < m_XBounds; j++)
					{
						// find the path between every other floor tile, in cases where no such path exists, the algorithm constructs new paths
						if (tileMap[i].index[j] == Tile::FloorTile)
						{
							FIndex first, second;
							first.x = x;
							first.y = y;
							second.x = j;
							second.y = i;
							newMap = BestFirstPathConstruction(first, second, newMap);
						}
					}
				}
			}
		}
	}

	return newMap;
}


TArray<FRow> MapGenerator::BestFirstPathConstruction(FIndex start, FIndex target, const TArray<FRow>& tileMap)
{
	// first of all find if a path exists
	bool pathExists{ false };
	TArray<FIndex> openList;
	TArray<FIndex> closedList;

	openList.Add(start);

	while (openList.Num() > 0 && pathExists == false)
	{
		// use the index with the lowest heuristic distance, in this case, manhattan heuristic will be used
		// the lowest number when the x difference and y difference between this index and the target index and are summed
		FIndex currentItem = openList[0];
		int32 removeIndex{ 0 };
		for (int i = 0; i < openList.Num(); i++)
		{
			int32 newManhattanHeuristic = (abs(openList[i].x - target.x)) + (abs(openList[i].y - target.y));
			int32 oldManhattanHeuristic = (abs(currentItem.x - target.x)) + (abs(currentItem.y - target.y));
			if (newManhattanHeuristic < oldManhattanHeuristic)
			{
				currentItem = openList[i];
				removeIndex = i;
			}
		}

		openList.RemoveAt(removeIndex);

		if (currentItem == target) {
			pathExists = true;
		}

		TArray<FIndex> adjacentTiles = GetNeighbours(currentItem);

		for (FIndex tile : adjacentTiles)
		{
			// only search directly adjacent tiles (above, below, left or right)
			if (tile.directlyAdjacent)
			{
				if (tile.x >= 0 && tile.x < m_XBounds && tile.y >= 0 && tile.y < m_YBounds)
				{
					if (tileMap[tile.y].index[tile.x] == Tile::FloorTile && !openList.Contains(tile) && !closedList.Contains(tile)) // if neighbour isnt in open or closed list and is a floor tile, traverse
					{
						openList.Add(tile);
					}
				}
			}
		}

		closedList.Add(currentItem);

	}

	if (pathExists)
	{
		return tileMap;
	}
	else
	{
		TArray<FRow> newMap = tileMap;

		// then from the closed list, find the index that was closest to the target and construct a best first path to the target
		FIndex currentItem = closedList[0];
		for (int i = 0; i < closedList.Num(); i++)
		{
			int32 newManhattanHeuristic = (abs(closedList[i].x - target.x)) + (abs(closedList[i].y - target.y));
			int32 oldManhattanHeuristic = (abs(currentItem.x - target.x)) + (abs(currentItem.y - target.y));
			if (newManhattanHeuristic < oldManhattanHeuristic)
			{
				currentItem = closedList[i];
			}
		}

		closedList.Empty();
		bool endReached{ false };
		int32 count{ 0 };
		while (!endReached && count < 5000)
		{
			// find neighbours of closest item, then use the closest one as the new closest item, converting it to a floor tile if necessary
			if (newMap[currentItem.y].index[currentItem.x] != Tile::FloorTile)
			{
				newMap[currentItem.y].index[currentItem.x] = Tile::FloorTile;
			}

			TArray<FIndex> adjacentTiles = GetNeighbours(currentItem);
			closedList.Add(currentItem); // add to new closed list
			currentItem = adjacentTiles[0];

			for (int i = 0; i < adjacentTiles.Num(); i++)
			{
				if (adjacentTiles[i].directlyAdjacent)
				{
					if (adjacentTiles[i] == target) {
						endReached = true;
						return newMap;
					}
					int32 newManhattanHeuristic = (abs(adjacentTiles[i].x - target.x)) + (abs(adjacentTiles[i].y - target.y));
					int32 oldManhattanHeuristic = (abs(currentItem.x - target.x)) + (abs(currentItem.y - target.y));
					if (newManhattanHeuristic < oldManhattanHeuristic)
					{
						currentItem = adjacentTiles[i];
					}
				}
			}
			count++;
		}
		return newMap;

	}

	
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
					if (y < m_Chunks[chunkY].chunks[chunkX].yBound)
					{
						if (x < m_Chunks[chunkY].chunks[chunkX].xBound)
						{
							newRow.index.Add(m_Chunks[chunkY].chunks[chunkX].chunkMap[y].index[x]);
						}
						else
						{
							newRow.index.Add(Tile::NoTile);
						}
					}
					else
					{
						newRow.index.Add(Tile::NoTile);
					}
				}

			}
			newMap.Add(newRow);
		}
	}


	return newMap;
}