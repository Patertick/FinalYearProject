// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile3D.generated.h"

UENUM()
enum TileType
{
	None     UMETA(DisplayName = "Nothing on this tile"),
	Wall      UMETA(DisplayName = "Wall on this tile"),
	Object   UMETA(DisplayName = "Object on this tile"),
	NPC   UMETA(DisplayName = "NPC on this tile"),
	Hazard   UMETA(DisplayName = "Hazard on this tile"),
};


UCLASS()
class PROJECTIMPETUS_API ATile3D : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile3D();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		UStaticMeshComponent* m_Mesh { nullptr };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	const float KMAXTILEDISTANCE{ 150.0f };
	TileType m_Type{ TileType::None };
	TArray<ATile3D*> m_connectedTiles;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = Type)
		void SetType(TileType newType) { m_Type = newType; }

	TileType GetType() { return m_Type; }

	TArray<ATile3D*> GetConnectedTiles() { return m_connectedTiles; }

};
