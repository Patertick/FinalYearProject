// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum Tile {
	FloorTile UMETA(DisplayName = "Floor Tile"),
	WallTile UMETA(DisplayName = "Wall Tile"),
	NoTile UMETA(DisplayName = "No Tile"),
};

class PROJECTIMPETUS_API MapGenerator
{
private:
	// map data, will be transformed into actual tiles on the map at runtime using level blueprint
	TArray<TEnumAsByte<Tile>> m_Map;
	// map bounds, randomly decided at begin
	int32 m_XBounds{ 0 };
	int32 m_YBounds{ 0 };

public:
	MapGenerator();
	~MapGenerator();

	TArray<TEnumAsByte<Tile>> GetTileMap() { return m_Map; }

	int32 GetXBounds() { return m_XBounds; }
	int32 GetYBounds() { return m_YBounds; }
	
};
