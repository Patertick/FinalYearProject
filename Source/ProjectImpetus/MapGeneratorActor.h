// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapGenerator.h"
#include "GameFramework/Actor.h"
#include "MapGeneratorActor.generated.h"

UCLASS()
class PROJECTIMPETUS_API AMapGeneratorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapGeneratorActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:
	MapGenerator* m_MapGen{ nullptr };
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = MapGen)
		TArray<FString> GetMapData() { return m_MapGen->GetTileMap(); }
	UFUNCTION(BlueprintCallable, Category = MapGen)
		int32 GetXBoundsData() { return m_MapGen->GetXBounds(); }
	UFUNCTION(BlueprintCallable, Category = MapGen)
		int32 GetYBoundsData() { return m_MapGen->GetYBounds(); }
	UFUNCTION(BlueprintCallable, Category = MapGen)
		float GetMapFitness() { return m_MapGen->GetMapTraversability(); }
};
