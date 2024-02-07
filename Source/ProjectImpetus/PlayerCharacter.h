// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PROJECTIMPETUS_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool m_DebugMode{ true };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	TArray<AActor*> m_SelectedActors;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// selected actor array functions
	void EmptySelectedActors() { m_SelectedActors.Empty(); }
	UFUNCTION(BlueprintCallable, Category = SelectedActors)
		TArray<AActor*> GetSelectedActors() { return m_SelectedActors; }
	UFUNCTION(BlueprintCallable, Category = SelectedActors)
		void AddSelectedActors(TArray<AActor*> newActors);
	UFUNCTION(BlueprintCallable, Category = SelectedActors)
		void SelectActor(AActor* actor);

};
