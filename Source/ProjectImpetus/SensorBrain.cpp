// Fill out your copyright notice in the Description page of Project Settings.


#include <Kismet/GameplayStatics.h>
#include "NPC.h"
#include "SensorBrain.h"

// Sets default values for this component's properties
USensorBrain::USensorBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USensorBrain::BeginPlay()
{
	Super::BeginPlay();
	// ...

	// get all actors in level
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
	m_ActorsInLevel = Actors;
	
}


// Called every frame
void USensorBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_NPCRef == nullptr) return;

	if (m_NPCRef->m_Blind == false)
	{
		// if rotation or location changes then the view changes
		// thus we calculate all the objects within this new view
		m_View.forwardVector = m_NPCRef->GetActorForwardVector();

		m_View.origin = m_NPCRef->GetActorLocation();

		//// reset seens
		//for (AActor* actor : m_ObjectsWithinView)
		//{
		//	// set material to some arbritrary material that denotes being seen

		//	if (Cast<ATile3D>(actor) != nullptr)
		//	{
		//		Cast<ATile3D>(actor)->m_IsSeen = false;
		//	}
		//	else if (Cast<AInteractable>(actor) != nullptr)
		//	{
		//		Cast<AInteractable>(actor)->m_IsSeen = false;
		//	}
		//	else if (Cast<ANPC>(actor) != nullptr)
		//	{
		//		Cast<ANPC>(actor)->m_IsSeen = false;
		//	}
		//}

		m_ObjectsWithinView.Empty();
		for (AActor* actor : m_ActorsInLevel)
		{
			if (IsInView(actor) && actor != m_NPCRef && IsNotObstructed(actor))
			{
				// add to objects within view if it is within view and not obstructed
				m_ObjectsWithinView.Add(actor);
			}
		}

		//// reset seens
		//for (AActor* actor : m_ObjectsWithinView)
		//{
		//	// set material to some arbritrary material that denotes being seen

		//	if (Cast<ATile3D>(actor) != nullptr)
		//	{
		//		Cast<ATile3D>(actor)->m_IsSeen = true;
		//	}
		//	else if (Cast<AInteractable>(actor) != nullptr)
		//	{
		//		Cast<AInteractable>(actor)->m_IsSeen = true;
		//	}
		//	else if (Cast<ANPC>(actor) != nullptr)
		//	{
		//		Cast<ANPC>(actor)->m_IsSeen = true;
		//	}
		//}

		// successful objects in view update

		// send message to update memory with new array of objects in view
		m_NPCRef->UpdateMemory(m_ObjectsWithinView);
	}

	// ...
}

bool USensorBrain::IsNotObstructed(AActor* actor)
{
	// line trace at actor to find if its within view

	// check top, bottom, front, back, left and right part of object

	// bottom
	FHitResult Hit;

	FVector actorVector = actor->GetActorLocation();

	FVector selfToActor = (actorVector - m_NPCRef->GetActorLocation()).GetSafeNormal();

	FVector TraceStart = m_NPCRef->GetActorLocation();
	TraceStart.Z += m_NPCRef->m_HalfHeight;
	FVector TraceEnd = actorVector + (selfToActor * m_View.maxDist);


	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(m_NPCRef);

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

	if (Hit.Actor == actor)
	{
		// no objects are in the way
		return true;
	}


	// top

	actorVector = actor->GetActorLocation();

	actorVector.Z += (ATile3D::TileSize() * actor->GetActorScale().Z);

	selfToActor = (actorVector - m_NPCRef->GetActorLocation()).GetSafeNormal();

	TraceStart = m_NPCRef->GetActorLocation();
	TraceStart.Z += m_NPCRef->m_HalfHeight;
	TraceEnd = actorVector + (selfToActor * m_View.maxDist);

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

	if (Hit.Actor == actor)
	{
		return true;
	}

	// front

	actorVector = actor->GetActorLocation();

	actorVector.Z += ((ATile3D::TileSize() * actor->GetActorScale().Z) / 2);
	actorVector.X += ((ATile3D::TileSize() * actor->GetActorScale().X) / 2);

	selfToActor = (actorVector - m_NPCRef->GetActorLocation()).GetSafeNormal();

	TraceStart = m_NPCRef->GetActorLocation();
	TraceStart.Z += m_NPCRef->m_HalfHeight;
	TraceEnd = actorVector + (selfToActor * m_View.maxDist);

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

	if (Hit.Actor == actor)
	{
		return true;
	}

	// back

	actorVector = actor->GetActorLocation();

	actorVector.Z += ((ATile3D::TileSize() * actor->GetActorScale().Z) / 2);
	actorVector.X -= ((ATile3D::TileSize() * actor->GetActorScale().X) / 2);

	selfToActor = (actorVector - m_NPCRef->GetActorLocation()).GetSafeNormal();

	TraceStart = m_NPCRef->GetActorLocation();
	TraceStart.Z += m_NPCRef->m_HalfHeight;
	TraceEnd = actorVector + (selfToActor * m_View.maxDist);

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

	if (Hit.Actor == actor)
	{
		return true;
	}

	// left

	actorVector = actor->GetActorLocation();

	actorVector.Z += ((ATile3D::TileSize() * actor->GetActorScale().Z) / 2);
	actorVector.Y -= ((ATile3D::TileSize() * actor->GetActorScale().Y) / 2);

	selfToActor = (actorVector - m_NPCRef->GetActorLocation()).GetSafeNormal();

	TraceStart = m_NPCRef->GetActorLocation();
	TraceStart.Z += m_NPCRef->m_HalfHeight;
	TraceEnd = actorVector + (selfToActor * m_View.maxDist);

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

	if (Hit.Actor == actor)
	{
		return true;
	}

	// right

	actorVector = actor->GetActorLocation();

	actorVector.Z += ((ATile3D::TileSize() * actor->GetActorScale().Z) / 2);
	actorVector.Y += ((ATile3D::TileSize() * actor->GetActorScale().Y) / 2);

	selfToActor = (actorVector - m_NPCRef->GetActorLocation()).GetSafeNormal();

	TraceStart = m_NPCRef->GetActorLocation();
	TraceStart.Z += m_NPCRef->m_HalfHeight;
	TraceEnd = actorVector + (selfToActor * m_View.maxDist);

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

	if (Hit.Actor == actor)
	{
		return true;
	}

	// objects are in the way
	return false;
}

bool USensorBrain::IsInView(AActor* actor)
{
	// find angle between this NPC's forward vector and target actors location, if the angle is higher than FOV, not within view
	// then find the overall distance, if this exceeds view distance, then the actor is not within view

	FVector2D forwardVector2D = FVector2D{ m_View.forwardVector.X, m_View.forwardVector.Y };
	FVector SelfToActor = actor->GetActorLocation() - m_View.origin;
	FVector2D SelfToActor2D = FVector2D{ SelfToActor.X, SelfToActor.Y }.GetSafeNormal();

	float angle = FVector2D::DotProduct(forwardVector2D, SelfToActor2D);
	if (angle < 0.0f) return false; // cannot acos a minus number

	angle = acosf(angle);
	angle = (angle * 180.0f) / PI;

	if (angle <= m_View.fov)
	{
		if (SelfToActor.Size() <= m_View.maxDist)
		{
			
			// within fov and view distance, thus it is within view
			return true;
		}
	}
	return false;
}
