// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile3D.h"
#include <Kismet/GameplayStatics.h>
#include <string>

// Sets default values
ATile3D::ATile3D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	m_Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	

}

// Called when the game starts or when spawned
void ATile3D::BeginPlay()
{
	Super::BeginPlay();

	

}

void ATile3D::FindConnectedTiles()
{
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);

	// find adjacent tiles (within 150 units counts as adjacent)
	for (AActor* tile : Tiles)
	{
		if (tile != this && FVector::Distance(tile->GetActorLocation(), GetActorLocation()) <= KMAXTILEDISTANCE)
		{
			// adjacent
			m_connectedTiles.Add(Cast<ATile3D>(tile));
		}
	}
}

// Called every frame
void ATile3D::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);
	/*int count{ 0 };
	if (m_Type == TileType::NPC)
	{
		FString tempString = "NPC" + GetName();
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, *tempString);
	}*/

}

