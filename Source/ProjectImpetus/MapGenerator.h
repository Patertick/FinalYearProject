// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum Tile {
	FloorTile UMETA(DisplayName = "Floor Tile"),
	WallTile UMETA(DisplayName = "Wall Tile"),
	NoTile UMETA(DisplayName = "No Tile"),
};

struct FRow {

	TArray<TEnumAsByte<Tile>> index; // the x indices of the row
};

struct FIndex {

	int32 x;
	int32 y;

	bool operator==(const FIndex& other) const
	{
		if (x == other.x && y == other.y) return true;
		return false;
	}
};
class PROJECTIMPETUS_API MapGenerator
{
private:
	// map data, will be transformed into actual tiles on the map at runtime using level blueprint
	TArray<FRow> m_Map;

	// map bounds, randomly decided at begin
	int32 m_XBounds{ 0 };
	int32 m_YBounds{ 0 };

	float m_MapTraversability{ 0.0f };

public:
	MapGenerator();
	~MapGenerator();

	float GetMapTraversability(const TArray<FRow>& tileMap);
	bool CanTraverseBreadthFirstSearch(FIndex first, FIndex second, const TArray<FRow>& tileMap);
	TArray<FIndex> GetNeighbours(FIndex passedIndex);

	void GenerateMap();

	TArray<FString> GetTileMap() {
		TArray<FString> printMap;
		for (int y = 0; y < m_YBounds; y++)
		{
			FString mapCol = "";
			for (int x = 0; x < m_XBounds; x++)
			{
				if (m_Map[y].index[x] == Tile::FloorTile)
				{
					mapCol = mapCol + "F";
				}
				else if (m_Map[y].index[x] == Tile::WallTile)
				{
					mapCol = mapCol + "W";
				}
				else
				{
					mapCol = mapCol + "N";
				}
			}
			printMap.Add(mapCol);
		}

		return printMap;
	}

	int32 GetXBounds() { return m_XBounds; }
	int32 GetYBounds() { return m_YBounds; }

	float GetSavedMapTraversability() { return m_MapTraversability; }
	
};
