// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"

// Sets default values
ANPC::ANPC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_Controllable = !m_Threat;

	switch (m_Directive)
	{
	case Directive::AttackThis:
		GEngine->AddOnScreenDebugMessage(ID, 15.0f, FColor::White, "I am attacking");
		break;
	case Directive::FollowThis:
		GEngine->AddOnScreenDebugMessage(ID, 15.0f, FColor::White, "I am following");
		break;
	case Directive::InteractThis:
		GEngine->AddOnScreenDebugMessage(ID, 15.0f, FColor::White, "I am interacting");
		break;
	case Directive::MoveHere:
		GEngine->AddOnScreenDebugMessage(ID, 15.0f, FColor::White, "I am moving");
		break;
	case Directive::DoNothing:
	default:
		GEngine->AddOnScreenDebugMessage(ID, 15.0f, FColor::White, "I am doing nothing");
		break;
	}

}

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

