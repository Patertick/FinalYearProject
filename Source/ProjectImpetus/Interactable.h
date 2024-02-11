// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

class UStaticMeshComponent;

class ANPC;

UCLASS()
class PROJECTIMPETUS_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		UStaticMeshComponent* m_Mesh { nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool m_IsSeen{ false };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Interact(ANPC* interactingNPC);

};
