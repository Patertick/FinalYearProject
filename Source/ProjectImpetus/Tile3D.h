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
	Escape   UMETA(DisplayName = "Can escape compound from this tile"),
};

UENUM()
enum FloorType
{
	HallwayFloor UMETA(DisplayName = "Hallway floor tile"),
	BreakRoomFloor UMETA(DisplayName = "Break Room floor tile"),
	MeetingRoomFloor UMETA(DisplayName = "Meeting Room floor tile"),
	ReceptionFloor UMETA(DisplayName = "Reception floor tile"),
	OfficeFloor UMETA(DisplayName = "Office floor tile"),
	CellFloor UMETA(DisplayName = "Cell floor tile"),
	ResearchRoomFloor UMETA(DisplayName = "Research Room floor tile"),
	BrokenWallFloor UMETA(DisplayName = "Broken wall floor tile"),
	DirtTile UMETA(DisplayName = "Dirt floor tile"),
	NotAFloor UMETA(DisplayName = "Not a floor tile"),
};

struct ConnectedTile {
	ATile3D* ref;
	float xDir; // between -1 and 1 (-1 being left and 1 being right)
	float yDir; // between -1 and 1 (-1 being up and 1 being down)
	FString direction;
};

class ANPC;


static const float KTILESIZE{ 100.0f };

UCLASS()
class PROJECTIMPETUS_API ATile3D : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile3D();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		UStaticMeshComponent* m_Mesh { nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool m_IsSeen{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloorProperties)
		TEnumAsByte<FloorType> m_FloorType { FloorType::NotAFloor };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloorProperties)
		bool m_StartedAsFloor{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attacking)
		bool m_Attacked{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attacking)
		UMaterialInterface* m_AttackMaterial { nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attacking)
		UMaterialInterface* m_DefaultMaterial { nullptr };
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	const float KMAXTILEDISTANCE{ 150.0f };
	TileType m_Type{ TileType::None };
	ANPC* m_NPCOnTile{ nullptr };
	TArray<ConnectedTile> m_connectedTiles;
	float m_Weight{ 1.0f };

	float m_AttackMaterialTimer{ 1.0f };
	const float KMAXATTACKTIMER{ 1.0f };

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = Type)
		void SetType(TileType newType) { m_Type = newType; }
	UFUNCTION(BlueprintCallable, Category = Startup)
		void FindConnectedTiles();
	UFUNCTION(BlueprintCallable, Category = Get)
		TEnumAsByte<TileType> GetType() { return m_Type; }

	UFUNCTION(BlueprintCallable, Category = Turn)
		void TurnToWall();
	UFUNCTION(BlueprintCallable, Category = Turn)
		void TurnToFloor();
	UFUNCTION(BlueprintCallable, Category = ResetTile)
		void Reset() {
		if (m_StartedAsFloor) {
			TurnToFloor();
			if (m_FloorType == FloorType::DirtTile)
			{
				m_Type = TileType::Escape;
			}
			else
			{
				m_Type = TileType::None;
			}
		}
		else {
			TurnToWall();
			m_Type = TileType::Wall;
		}
	}

	void AttackTile(ANPC* attackingNPC);

	TArray<ConnectedTile> GetConnectedTiles() { return m_connectedTiles; }

	void SetNPCOnTile(ANPC* npcOnTile) { m_NPCOnTile = npcOnTile; }

	float GetWeight() { return m_Weight; }

	static float TileSize() { return KTILESIZE; }

};
