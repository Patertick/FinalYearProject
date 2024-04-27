// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile3D.h"
#include <Kismet/GameplayStatics.h>
#include <string>
#include "NPC.h"
#include <Operations/GroupTopologyDeformer.h>

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

void ATile3D::TurnToFloor()
{
	SetActorRelativeScale3D(FVector{ 1.0f, 1.0f, 1.0f });
}

void ATile3D::TurnToWall()
{
	SetActorRelativeScale3D(FVector{ 1.0f, 1.0f, 5.0f });
}

void ATile3D::AttackTile(ANPC* attackingNPC)
{
	// set as attacked
	if (attackingNPC->GetHasDied()) return; // do not attack if dead

	m_Attacked = true;
	// find if an NPC is on the tile, if so, they are dealt damage

	if (m_NPCOnTile != nullptr)
	{
		if (m_NPCOnTile->GetHasDied()) return;
		// deal damage
		if (m_NPCOnTile->CanTakeDamage())
		{
			UGameplayStatics::ApplyDamage(m_NPCOnTile, attackingNPC->GetDamage(), nullptr, attackingNPC, NULL);
			attackingNPC->SetDamageDealt(attackingNPC->GetDamage());
			m_NPCOnTile->TakenDamage();
		}
	}

	m_Mesh->SetMaterial(0, m_AttackMaterial);
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
			ConnectedTile newTile;
			newTile.ref = Cast<ATile3D>(tile);
			FVector directionVector = tile->GetActorLocation() - GetActorLocation();
			directionVector.Normalize();
			newTile.xDir = directionVector.X;
			newTile.yDir = directionVector.Y;
			
			if (newTile.xDir == -1 && newTile.yDir == 0)
			{
				newTile.direction = "l";
			}
			else if (newTile.xDir == 1 && newTile.yDir == 0)
			{
				newTile.direction = "r";
			}
			else if (newTile.xDir == 0 && newTile.yDir == -1)
			{
				newTile.direction = "u";
			}
			else if (newTile.xDir == 0 && newTile.yDir == 1)
			{
				newTile.direction = "d";
			}
			else
			{
				newTile.direction = "NULL";
			}

			m_connectedTiles.Add(newTile);
		}
	}
}

// Called every frame
void ATile3D::Tick(float DeltaTime)
{
	if (m_Attacked)
	{
		Super::Tick(DeltaTime);
		m_AttackMaterialTimer -= DeltaTime;
		if (m_AttackMaterialTimer <= 0.0f)
		{
			m_Attacked = false;
			m_Mesh->SetMaterial(0, m_DefaultMaterial);
			m_AttackMaterialTimer = KMAXATTACKTIMER;
		}
	}

}

