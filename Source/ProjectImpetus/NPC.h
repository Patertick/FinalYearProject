// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPC.generated.h"

UENUM()
enum Directive
{
	MoveHere     UMETA(DisplayName = "MoveHere"),
	FollowThis      UMETA(DisplayName = "FollowThis"),
	AttackThis   UMETA(DisplayName = "AttackThis"),
	InteractThis   UMETA(DisplayName = "InteractThis"),
	DoNothing   UMETA(DisplayName = "DoNothing"),
};

UCLASS()
class PROJECTIMPETUS_API ANPC : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPC();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Identifiers)
		int32 ID { 0 };

	// perception properties, as in what is this NPC perceived to be
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PerceptionProperties)
		bool m_Threat{ false };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	Directive m_Directive{Directive::DoNothing };
	bool m_Controllable{ false };

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// player control functions
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		void SetDirective(Directive newDirective) { m_Directive = newDirective; }
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		void MarkAsThreat() { m_Threat = true; }
	UFUNCTION(BlueprintCallable, Category = PlayerControl)
		bool IsControllable() { return m_Controllable; }

};
