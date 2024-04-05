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

struct ConvexHull{
	// Edges
	TArray<Edge> edges;
	TArray<FVector> points;
};

struct Mesh {
	TArray<Triangle> triangles;
	FVector centroid;
};

struct Triangle {
	FVector a;
	FVector b;
	FVector c;

	FVector GetPlaneNormal()
	{

	}
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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
	FVector m_Centroid;
	const int32 KNUMBEROFCLOSESTPOINTS{ 4 };


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CreateNewMesh(UStaticMeshComponent* mesh);

	TArray<Face> ConstructFaces();

	int32 FindIndex(FVector vector);

	ConvexHull ConstructHull(const TArray<FVector>& points);

	Mesh ConstructMesh(const TArray<FVector>& points);

	bool IsMeshValid(const Mesh& mesh);
};
