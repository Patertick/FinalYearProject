// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SensorBrain.generated.h"


struct View {
	FVector origin;
	FVector forwardVector;
	float fov;
	float maxDist;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTIMPETUS_API USensorBrain : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USensorBrain();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	ANPC* m_NPCRef{ nullptr }; // reference to the object this brain works for
	TArray<AActor*> m_ObjectsWithinView;
	View m_View;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = SetNPC)
		void SetNPC(ANPC* npc) { m_NPCRef = npc; }

	TArray<AActor*> GetAllObjectsWithinView() { return m_ObjectsWithinView; }
	void SetFieldOfView(float newFOV) { m_View.fov = newFOV; }
	void SetMaxViewDistance(float newDistance) { m_View.maxDist = newDistance; }
	bool IsNotObstructed(AActor* actor);
	bool IsInView(AActor* actor);
	bool IsNPCInView();

		
};
