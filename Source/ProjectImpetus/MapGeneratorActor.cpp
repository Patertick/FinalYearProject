// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorActor.h"

// Sets default values
AMapGeneratorActor::AMapGeneratorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMapGeneratorActor::BeginPlay()
{
	Super::BeginPlay();

	m_MapGen = new MapGenerator();

	// test map traversability
	
}

// Called every frame
void AMapGeneratorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (!m_FinishedGen)
	{
		m_MapGen->GenerateMap();
		m_FinishedGen = m_MapGen->HasMapFinished();
	}
	else
	{
		delete m_MapGen;
	}

}

