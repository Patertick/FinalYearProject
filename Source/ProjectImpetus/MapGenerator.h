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

struct FChunk {
	TArray<FRow> chunkMap;
	bool completed{ false };
	int32 xBound{ 0 };
	int32 yBound{ 0 };
};

struct FChunkRow
{
	TArray<FChunk> chunks;
};

struct FIndex {

	int32 x;
	int32 y;
	bool directlyAdjacent;

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

	TArray<FChunkRow> m_Chunks;

	// map bounds, randomly decided at begin
	int32 m_XBounds{ 0 };
	int32 m_YBounds{ 0 };

	// number of chunks on x and y axis
	int32 m_NumXChunks{ 0 };
	int32 m_NumYChunks{ 0 };

	float m_MapTraversability{ 0.0f };

	bool m_MapGenerated{ false };

public:
	MapGenerator();
	~MapGenerator();

	float GetMapTraversability(const TArray<FRow>& tileMap);
	bool CanTraverseBreadthFirstSearch(FIndex first, FIndex second, const TArray<FRow>& tileMap);
	TArray<FRow> ConstructPath(const TArray<FRow>& tileMap); // pass in a map and a map with constructed paths will be output

	TArray<FRow> FinalRefinement(const TArray<FRow>& tileMap, int32 xBounds, int32 yBounds);

	// if a path does not exist, by using best first searching, a direct path can be constructed to the goal
	// after which the final connected map will be refined so that floor tiles aren't exposed and wall tiles aren't redundant
	TArray<FRow> BestFirstPathConstruction(FIndex start, FIndex target, const TArray<FRow>& tileMap);

	TArray<FIndex> GetNeighbours(FIndex passedIndex);

	void RefineMap();

	FChunk RefineChunk(const FChunk& tileMap); // refine map iteratively using grammars with conditions

	TArray<FRow> ConcatenateChunksIntoMap(); // using chunks array, concatenate all chunks into a map that can translate to the game world

	void ConstructPaths(); // take final map and use best first search to construct paths between every floor tile

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

	bool HasMapFinished() { return m_MapGenerated; }
	
};
