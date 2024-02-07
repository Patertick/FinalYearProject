// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerCharacter::AddSelectedActors(TArray<AActor*> newActors)
{
	// add all new actors after emptying array
	EmptySelectedActors();
	for (int i = 0; i < newActors.Num(); i++)
	{
		m_SelectedActors.Add(newActors[i]);
	}
}

void APlayerCharacter::SelectActor(AActor* actor)
{
	// make sure there is only one actor in the array after this function completes
	EmptySelectedActors();
	m_SelectedActors.Add(actor);
	// add on screen message if debug mode is active and array is not of expected size
	if (m_SelectedActors.Num() != 1 && m_DebugMode) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("ERROR, ARRAY IS NOT OF SIZE 1"));
	}
}

