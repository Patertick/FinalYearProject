// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator.h"
#include "Math/UnrealMathUtility.h"

MapGenerator::MapGenerator()
{
	// randomize bounds
	m_XBounds = FMath::RandRange(10, 30);
	m_YBounds = FMath::RandRange(10, 30);
}

MapGenerator::~MapGenerator()
{
}

float MapGenerator::GetMapTraversability(const TArray<TEnumAsByte<Tile>>& tileMap)
{
	float traversableTiles{ 0 };
	float totalPossibleTraversableTiles{ 0 };

	// for each floor tile, find all floor tiles excluding self and see if they can be pathed to, if so add to fitness
	for (int row = 0; row < m_YBounds; row++)
	{
		for (int col = 0; col < m_YBounds; col++)
		{
			if (tileMap[row + col] == Tile::FloorTile)
			{
				for (int y = 0; y < m_YBounds; y++)
				{
					for (int x = 0; x < m_XBounds; x++)
					{
						if (tileMap[y + x] == Tile::FloorTile && y + x != row + col) // remember to make sure we don't test traversability on self
						{
							if (CanTraverseBreadthFirstSearch(y + x, row + col, tileMap))
							{
								traversableTiles++;
							}
							totalPossibleTraversableTiles++;
						}
					}
				}
			}
		}
	}


	return traversableTiles / totalPossibleTraversableTiles; // where 1.0f is a fully traversable map and 0.0f is a fully non traversable map
}

bool MapGenerator::CanTraverseBreadthFirstSearch(int32 first, int32 second, const TArray<TEnumAsByte<Tile>>& tileMap)
{
	TArray<int32> openList;
	TArray<int32> closedList;

	openList.Add(first);

	// breadth first search
	while (openList.Num() > 0)
	{
		int32 currentItem = openList[0];
		openList.RemoveAt(0);

		if (currentItem == second) {
			return true;
		}

		TArray<int32> adjacentTiles = GetNeighbours(currentItem);

		for (int32 tile : adjacentTiles)
		{
			if (tile >= 0 && tile < tileMap.Num())
			{
				if (tileMap[tile] == Tile::FloorTile && !openList.Contains(tile) && !closedList.Contains(tile)) // if neighbour isnt in open or closed list and is a floor tile, traverse
				{
					openList.Add(tile);
				}
			}
		}

		closedList.Add(currentItem);

	}


	return false;
}

TArray<int32> MapGenerator::GetNeighbours(int32 index)
{
	TArray<int32> neighbours;
	int32 newNeighbour;

	// top middle

	newNeighbour = (index - m_XBounds);
	if (index % m_XBounds != 0)
	{
		neighbours.Add(newNeighbour);
	}

	// left

	newNeighbour = index - 1;
	if (index % m_XBounds != 0)
	{
		neighbours.Add(newNeighbour);
	}

	// right

	newNeighbour = index + 1;
	if (index % m_XBounds != 0)
	{
		neighbours.Add(newNeighbour);
	}

	// bottom middle

	newNeighbour = (index + m_XBounds);
	if (index % m_XBounds != 0)
	{
		neighbours.Add(newNeighbour);
	}

	return neighbours;
}

void MapGenerator::GenerateMap()
{
	TArray<TEnumAsByte<Tile>> generatedMap;
	for (int y = 0; y < m_YBounds; y++)
	{
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
	}
	float mapTraversability = GetMapTraversability(generatedMap);
	if (m_MapTraversability < mapTraversability)
	{
		m_Map = generatedMap;
		m_MapTraversability = mapTraversability;
	}
}
