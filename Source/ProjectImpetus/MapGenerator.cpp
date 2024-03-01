// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator.h"
#include "Math/UnrealMathUtility.h"

MapGenerator::MapGenerator()
{
	// randomize bounds
	m_XBounds = FMath::RandRange(10, 30);
	m_YBounds = FMath::RandRange(10, 30);
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

		m_Map.Add(newRow);
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
	neighbours.Add(newNeighbour);
	

	// left

	newNeighbour.x = passedIndex.x - 1;
	newNeighbour.y = passedIndex.y;
	neighbours.Add(newNeighbour);

	// right

	newNeighbour.x = passedIndex.x + 1;
	newNeighbour.y = passedIndex.y;
	neighbours.Add(newNeighbour);

	// bottom

	newNeighbour.x = passedIndex.x;
	newNeighbour.y = passedIndex.y + 1;
	neighbours.Add(newNeighbour);

	return neighbours;
}

void MapGenerator::GenerateMap()
{
	TArray<FRow> generatedMap = m_Map;

	// change five random tiles

	for (int i = 0; i < 5; i++)
	{
		int32 randomXIndex = FMath::RandRange(0, m_XBounds - 1);
		int32 randomYIndex = FMath::RandRange(0, m_YBounds - 1);
		int32 randomTile = FMath::RandRange(0, 2);

		if (randomTile == 0)
		{
			generatedMap[randomYIndex].index[randomXIndex] = Tile::FloorTile;
		}
		else if (randomTile == 1)
		{
			generatedMap[randomYIndex].index[randomXIndex] = Tile::WallTile;
		}
		else
		{
			generatedMap[randomYIndex].index[randomXIndex] = Tile::NoTile;
		}

	}

	float mapTraversability = GetMapTraversability(generatedMap);
	if (m_MapTraversability < mapTraversability)
	{
		m_Map = generatedMap;
		m_MapTraversability = mapTraversability;
	}
}
