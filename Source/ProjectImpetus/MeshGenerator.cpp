// Fill out your copyright notice in the Description page of Project Settings.


#include "../../../Unreal/UE_4.27/Engine/Plugins/Experimental/GeometryProcessing/Source/MeshConversion/Public/MeshDescriptionBuilder.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "MeshGenerator.h"

// Sets default values for this component's properties
UMeshGenerator::UMeshGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	//m_GeneratedStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Generated Mesh"));

	// generate random set of points

	while ((m_XBounds * m_YBounds * m_ZBounds) % 3 != 0)
	{
		m_XBounds = FMath::RandRange(10, 20);
		m_YBounds = FMath::RandRange(10, 20);
		m_ZBounds = FMath::RandRange(10, 20);
	}
	for (int z = 0; z < m_ZBounds; z++)
	{
		for (int y = 0; y < m_YBounds; y++)
		{
			for (int x = 0; x < m_XBounds; x++)
			{
				float newValX = FMath::FRandRange(0.0f, 100.0f); // x co ord
				float newValY = FMath::FRandRange(0.0f, 100.0f); // y co ord
				float newValZ = FMath::FRandRange(0.0f, 100.0f); // z co ord
				FVector vec = FVector{ newValX, newValY, newValZ };
				m_Mesh.Add(vec);
			}
		}
	}
}


// Called when the game starts
void UMeshGenerator::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UMeshGenerator::CreateNewMesh(UStaticMeshComponent* mesh)
{
	// mesh description (holds uv values, normals etc.
	FMeshDescription meshDesc;
	FStaticMeshAttributes Attributes(meshDesc);
	Attributes.Register();

	FMeshDescriptionBuilder meshDescBuilder;
	meshDescBuilder.SetMeshDescription(&meshDesc);
	meshDescBuilder.EnablePolyGroups();
	meshDescBuilder.SetNumUVLayers(1);

	// create vertices from m_Mesh vector values
	TArray<FVertexID> vertexIDs;
	vertexIDs.SetNum(m_XBounds * m_YBounds * m_ZBounds);
	for (int z = 0; z < m_ZBounds; z++)
	{
		for (int y = 0; y < m_YBounds; y++)
		{
			for (int x = 0; x < m_XBounds; x++)
			{
				vertexIDs[IndexFromXYZ(x, y, z)] = meshDescBuilder.AppendVertex(m_Mesh[IndexFromXYZ(x, y, z)]);
			}
		}
	}

	// array that stores vertex instances (3 vertices per face)
	TArray<FVertexInstanceID> vertexInsts;

	// create faces

	TArray<Face> faces = ConstructFaces();



	// using faces as reference, assign vertex instances
	for (Face face : faces)
	{
		// find the three vertices in vertexIDs that are apart of the face

		FVertexInstanceID instanceA, instanceB, instanceC;
		FVector vectorA, vectorB, vectorC;
		int32 indexA, indexB, indexC;
		bool setValues{ false };
	
		// find three unqiue vectors from face edges

		while (!setValues)
		{
			indexA = FMath::RandRange(0, (m_XBounds * m_YBounds * m_ZBounds) - 1);
			indexB = FMath::RandRange(0, (m_XBounds * m_YBounds * m_ZBounds) - 1);
			indexC = FMath::RandRange(0, (m_XBounds * m_YBounds * m_ZBounds) - 1);
			if (m_Mesh[indexA].Equals(m_Mesh[indexB]) || m_Mesh[indexA].Equals(m_Mesh[indexC]) || m_Mesh[indexB].Equals(m_Mesh[indexC]))
			{
				setValues = false;
			}
			else
			{
				setValues = true;
			}
		}

		vectorA = m_Mesh[indexA];
		vectorB = m_Mesh[indexB];
		vectorC = m_Mesh[indexC];

		instanceA = meshDescBuilder.AppendInstance(vertexIDs[indexA]);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("A SET"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(indexA));

		instanceB = meshDescBuilder.AppendInstance(vertexIDs[indexB]);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("B SET"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(indexB));

		instanceC = meshDescBuilder.AppendInstance(vertexIDs[indexC]);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("C SET"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(indexC));

		// set normal
		// find the direction away from centroid for each vertex

		meshDescBuilder.SetInstanceNormal(instanceA, vectorA.GetSafeNormal());
		meshDescBuilder.SetInstanceNormal(instanceB, vectorB.GetSafeNormal());
		meshDescBuilder.SetInstanceNormal(instanceC, vectorC.GetSafeNormal());


		// set uv values (a is 0,1 b is 0,0 and c is 1,0)

		meshDescBuilder.SetInstanceUV(instanceA, FVector2D(0, 1));
		meshDescBuilder.SetInstanceUV(instanceB, FVector2D(0, 0));
		meshDescBuilder.SetInstanceUV(instanceC, FVector2D(1, 0));

		// set colour

		meshDescBuilder.SetInstanceColor(instanceA, FVector4(1.0f, 1.0f, 1.0f, 1.0f));
		meshDescBuilder.SetInstanceColor(instanceB, FVector4(1.0f, 1.0f, 1.0f, 1.0f));
		meshDescBuilder.SetInstanceColor(instanceC, FVector4(1.0f, 1.0f, 1.0f, 1.0f));

		// add to vertex instances

		vertexInsts.Add(instanceA);
		vertexInsts.Add(instanceB);
		vertexInsts.Add(instanceC);
	}

	// allocate polygon group
	FPolygonGroupID polygonGroup = meshDescBuilder.AppendPolygonGroup();

	// create triangles

	for (int i = 0; i < vertexInsts.Num(); i += 3)
	{
		meshDescBuilder.AppendTriangle(vertexInsts[i + 2], vertexInsts[i + 1], vertexInsts[i], polygonGroup);
	}

	// set default material
	UStaticMesh* staticMesh = NewObject<UStaticMesh>(this);
	staticMesh->GetStaticMaterials().Add(FStaticMaterial());

	UStaticMesh::FBuildMeshDescriptionsParams mdParams;
	mdParams.bBuildSimpleCollision = true;
	mdParams.bAllowCpuAccess = true;

	// build the static mesh
	TArray<const FMeshDescription*> meshDescPtrs;
	meshDescPtrs.Emplace(&meshDesc);
	staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);

	// Assign new static mesh to the static mesh component
	mesh->SetStaticMesh(staticMesh);
}

TArray<Face> UMeshGenerator::ConstructFaces()
{
	TArray<Face> faces;
	return faces;
}

// Called every frame
void UMeshGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

