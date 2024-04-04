// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeshGenerator.generated.h"

struct Face {
	FVector a;
	int32 aIndex;
	FVector b;
	int32 bIndex;
	FVector c;
	int32 cIndex;

	bool operator==(const Face other) const
	{
		if (a == other.a)
		{
			if(b == other.b)
			{
				if (c == other.c)
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
	int32 m_XBounds{ 0 };
	int32 m_YBounds{ 0 };
	int32 m_ZBounds{ 0 };

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CreateNewMesh(UStaticMeshComponent* mesh);

	TArray<Face> ConstructFaces();

	int32 IndexFromXYZ(int32 x, int32 y, int32 z) {
		return (z * m_YBounds) + (y * m_XBounds) + x;
	}
};
