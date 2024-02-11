// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable.h"
#include "Components/StaticMeshComponent.h"
#include "NPC.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AInteractable::AInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	m_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	m_Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);


}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	Super::BeginPlay();

	// find tile this object is currently on
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile3D::StaticClass(), Tiles);

	AActor* closestTile = nullptr;
	for (AActor* tile : Tiles)
	{

		FVector2D actorPos = FVector2D{ GetActorLocation().X, GetActorLocation().Y };
		FVector2D tilePos = FVector2D{ tile->GetActorLocation().X, tile->GetActorLocation().Y };
		if (closestTile == nullptr) closestTile = tile;
		else if (FVector2D::Distance(tilePos, actorPos) <
			FVector2D::Distance(FVector2D{ closestTile->GetActorLocation().X, closestTile->GetActorLocation().Y }, actorPos))
		{
			closestTile = tile;
		}
	}

	if (Cast<ATile3D>(closestTile) != nullptr)
	{
		Cast<ATile3D>(closestTile)->SetType(TileType::Object);
	}

}

// Called every frame
void AInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	
}

void AInteractable::Interact(ANPC* interactingNPC)
{
	GEngine->AddOnScreenDebugMessage(5, 5.0f, FColor::White, TEXT("I have been interacted with"));
}
