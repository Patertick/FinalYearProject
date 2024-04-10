// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeshGenerator.generated.h"

struct Edge {
	FVector a;
	FVector b;

	bool operator==(const Edge other) const
	{
		if (a == other.a)
		{
			if (b == other.b)
			{
				return true;
			}
		}
		else if (a == other.b)
		{
			if (b == other.a)
			{
				return true;
			}
		}
		return false;
	}
};

struct ConvexHull {
	// Edges
	TArray<Edge> edges;
	TArray<FVector> points;
};

struct Plane {
	FVector normal;
	FVector pointOnPlane;

	bool IsInFrontOfPlane(FVector vec) // DO NOT MARK: THIS WORK WAS TAKEN FROM PAST PROJECT
	{
		FVector P = vec - pointOnPlane;

		float dist = FVector::DotProduct(normal, P) / P.Size();

		if (dist > 0.0f) return true; // point is in front of plane
		return false; // point is behind or on plane
	}
};

struct Triangle {
	FVector a;
	FVector b;
	FVector c;

	TArray<Triangle> connectedTriangles;

	FVector GetPlaneNormal()
	{
		return FVector::CrossProduct(a, b).GetSafeNormal();
	}

	bool Intersects(const Triangle& other)
	{
		return false;
	}

	bool operator==(const Triangle other) const
	{
		if (a == other.a)
		{
			if (b == other.b)
			{
				if (c == other.c)
				{
					return true;
				}
			}
			else if (b == other.c)
			{
				if (c == other.b)
				{
					return true;
				}
			}
		}
		else if (a == other.b)
		{
			if (b == other.a)
			{
				if (c == other.c)
				{
					return true;
				}
			}
			else if (b == other.c)
			{
				if (c == other.a)
				{
					return true;
				}
			}
		}
		else if (a == other.c)
		{
			if (b == other.a)
			{
				if (c == other.b)
				{
					return true;
				}
			}
			else if (b == other.b)
			{
				if (c == other.a)
				{
					return true;
				}
			}
		}
		return false;
	}

};

struct Mesh {
	TArray<Triangle> triangles;
	FVector centroid;
};

struct Face {
	int32 aIndex;
	int32 bIndex;
	int32 cIndex;

	bool operator==(const Face other) const
	{
		if (aIndex == other.aIndex)
		{
			if (bIndex == other.bIndex)
			{
				if (cIndex == other.cIndex)
				{
					return true;
				}
			}
			else if (bIndex == other.cIndex)
			{
				if (cIndex == other.bIndex)
				{
					return true;
				}
			}
		}
		else if (aIndex == other.bIndex)
		{
			if (bIndex == other.aIndex)
			{
				if (cIndex == other.cIndex)
				{
					return true;
				}
			}
			else if (bIndex == other.cIndex)
			{
				if (cIndex == other.aIndex)
				{
					return true;
				}
			}
		}
		else if (aIndex == other.cIndex)
		{
			if (bIndex == other.aIndex)
			{
				if (cIndex == other.bIndex)
				{
					return true;
				}
			}
			else if (bIndex == other.bIndex)
			{
				if (cIndex == other.aIndex)
				{
					return true;
				}
			}
		}
		return false;
	}
};

struct Column {
	TArray<bool> col;
};

struct Row {
	TArray<Column> row;
};

struct Index {
	int32 x;
	int32 y;
	int32 z;

	void operator=(const Index other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
	bool operator==(const Index other) const
	{
		if (x == other.x)
		{
			if (y == other.y)
			{
				if (z == other.z)
				{
					return true;
				}
			}
		}
		return false;
	}
	bool operator!=(const Index other) const
	{
		if (x == other.x)
		{
			if (y == other.y)
			{
				if (z == other.z)
				{
					return false;
				}
			}
		}
		return true;
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIMPETUS_API UMeshGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMeshGenerator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	TArray<FVector> m_Mesh;
	TArray<Row> m_MeshMap;
	FVector m_Centroid;
	int32 m_ZBounds;
	int32 m_YBounds;
	int32 m_XBounds;
	const float KSIZEOFCUBE{ 10.0f };
	const int32 KNUMBEROFCLOSESTPOINTS{ 4 };
	const float KMAXDISTANCE{ 10.0f };
	const float KMINDISTANCE{ 5.0f };


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CreateNewMesh(UStaticMeshComponent* mesh);

	TArray<Face> ConstructFaces();

	int32 FindIndex(FVector vector);

	ConvexHull ConstructHull(const TArray<FVector>& points);

	Mesh ConstructMesh(const TArray<FVector>& points);

	bool IsMeshValid(const Mesh& mesh);

	Triangle GenerateTriangle(FVector direction, FVector firstPoint = FVector{}, FVector secondPoint = FVector{});

	TArray<FVector> GenerateCube(FVector centrePoint, FVector offset);

	TArray<Index> GetAdjacentItems(Index index);

	Index GetRandomCube();

	bool IsIndexConnected(Index start, Index end);

	int32 IndexFromXYZ(int32 x, int32 y, int32 z) {
		return (z * m_YBounds) + (y * m_XBounds) + x;
	}
};
