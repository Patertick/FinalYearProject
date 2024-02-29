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
		for (int x = 0; x < m_XBounds; x++)
		{
			int32 randomTile = FMath::RandRange(0, 2);

			if (randomTile == 0)
			{
				m_Map.Add(Tile::FloorTile);
			}
			else if (randomTile == 1)
			{
				m_Map.Add(Tile::WallTile);
			}
			else
			{
				m_Map.Add(Tile::NoTile);
			}
		}
	}
}

MapGenerator::~MapGenerator()
{
}
