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

	// ensure no points overlap & that number of points is a multiple of 3

	int32 numberOfPoints{ 0 };
	do {
		numberOfPoints = FMath::RandRange(1000, 10000);
	} while (numberOfPoints % 3 != 0);

	m_Centroid = FVector{ 0.0f, 0.0f, 0.0f };

	bool validMesh{ false };

	while (!validMesh)
	{
		TArray<FVector> genMeshPointSet;
		for (int i = 0; i < numberOfPoints; i++)
		{
			float newValX = FMath::FRandRange(-100.0f, 100.0f); // x co ord
			float newValY = FMath::FRandRange(-100.0f, 100.0f); // y co ord
			float newValZ = FMath::FRandRange(-100.0f, 100.0f); // z co ord
			FVector vec = FVector{ newValX, newValY, newValZ };
			genMeshPointSet.Add(vec);
			m_Centroid += vec;
		}

		m_Centroid /= numberOfPoints;

		Mesh mesh = ConstructMesh(genMeshPointSet);
		if (IsMeshValid(mesh))
		{
			m_Mesh = genMeshPointSet;
			validMesh = true;
		}
	}
}

Mesh UMeshGenerator::ConstructMesh(const TArray<FVector>& points)
{
	Mesh newMesh;
	for (int i = 0; i < m_Mesh.Num(); i+=3)
	{
		Triangle newTriangle;

		newTriangle.a = m_Mesh[i + 2];
		newTriangle.b = m_Mesh[i + 1];
		newTriangle.c = m_Mesh[i];

		newMesh.triangles.Add(newTriangle);
	}

	newMesh.centroid = m_Centroid;

	return newMesh;
}

bool UMeshGenerator::IsMeshValid(const Mesh& mesh)
{
	for (Triangle tri : mesh.triangles)
	{
		int32 numberOfTrianglesThatSharesTwoPoints{ 0 };
		// check each triangle against other triangles
		for (Triangle other : mesh.triangles)
		{
			// if the other triangle is completely on either side of the current triangle, there is not an intersection

			//
		}


		// if this triangle doesn't share two points with 3 other triangles, it must be disjoint
		if (numberOfTrianglesThatSharesTwoPoints < 3)
		{
			return false;
		}

		// if the cross products of the points is not away from the centroid, then it must be facing the wrong direction
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
	vertexIDs.SetNum(m_Mesh.Num());
	for (int i = 0; i < m_Mesh.Num(); i++)
	{
		vertexIDs[i] = meshDescBuilder.AppendVertex(m_Mesh[i]);
	}

	// array that stores vertex instances (3 vertices per face)
	TArray<FVertexInstanceID> vertexInsts;

	// using faces as reference, assign vertex instances
	for (int i = 0; i < vertexIDs.Num(); i+= 3)
	{
		// find the three vertices in vertexIDs that are apart of the face

		FVertexInstanceID instanceA = meshDescBuilder.AppendInstance(vertexIDs[i]);
		FVertexInstanceID instanceB = meshDescBuilder.AppendInstance(vertexIDs[i + 1]);
		FVertexInstanceID instanceC = meshDescBuilder.AppendInstance(vertexIDs[i + 2]);

		// set normal
		// find the direction away from centroid for each vertex

		meshDescBuilder.SetInstanceNormal(instanceA, (m_Mesh[i] - m_Centroid).GetSafeNormal());
		meshDescBuilder.SetInstanceNormal(instanceB, (m_Mesh[i + 1] - m_Centroid).GetSafeNormal());
		meshDescBuilder.SetInstanceNormal(instanceC, (m_Mesh[i + 2] - m_Centroid).GetSafeNormal());

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

	for (int i = 0; i < m_Mesh.Num(); i++)
	{
		// find the four closest points
		
		TArray<FVector> closestPoints;
		for (int j = 0; j < KNUMBEROFCLOSESTPOINTS; j++)
		{
			closestPoints.Add(m_Mesh[FMath::RandRange(0, m_Mesh.Num() - 1)]); // initialise with random values from set of points
		}

		for (int j = 0; j < m_Mesh.Num(); j++)
		{
			if (j == i) { continue; }
			else
			{
				for (int k = 0; k < closestPoints.Num(); k++)
				{
					// is current j point closer to i point than the current k entry in closest points?
					float newDistance = FVector::Distance(m_Mesh[j], m_Mesh[i]);
					float oldDistance = FVector::Distance(closestPoints[k], m_Mesh[i]);
					if (newDistance < oldDistance && !closestPoints.Contains(m_Mesh[j]))
					{
						// replace current k entry in closest points with j point
						closestPoints[k] = m_Mesh[j];
					}
				}
			}
		}

		// add the current point to the closest points array

		closestPoints.Add(m_Mesh[i]);

		// using these points, we need to construct faces such that ordering is correct

		// create a 2D convex hull from set of 5 points

		ConvexHull hull = ConstructHull(closestPoints);

		// find the point that can create the most edges to other points that do not lie on the convex hull

		FVector basePoint = FVector{};
		int32 highestNumberOfEdgesNotOnHull{ 0 };
		for (int j = 0; j < closestPoints.Num(); j++)
		{
			int32 numberOfEdgesNotOnHull{ 0 };
			for (int k = 0; k < closestPoints.Num(); k++)
			{
				if (j == k) { continue; }
				else
				{
					Edge edge;
					edge.a = closestPoints[j];
					edge.b = closestPoints[k];

					if (!hull.edges.Contains(edge))
					{
						numberOfEdgesNotOnHull++;

					}
				}
			}
			if (numberOfEdgesNotOnHull > highestNumberOfEdgesNotOnHull)
			{
				basePoint = closestPoints[j];
			}
			
		}



		// using this point, create faces such that all points are connected

		for (int j = 0; j < closestPoints.Num(); j++)
		{
			if (closestPoints[j].Equals(basePoint)) { continue; }
			else
			{
				Face newFace;

				// a is base point

				newFace.aIndex = FindIndex(basePoint);

				// b is current point

				newFace.bIndex = FindIndex(closestPoints[j]);

				// c is found by cross referencing hull edges (any edge that contains b, the other point is c)

				// once an edge is used in this algorithm its removed from the hull so that all edges are used to make faces

				newFace.cIndex = -1;

				for (int k = 0; k < hull.edges.Num(); k++)
				{
					if (newFace.cIndex == -1)
					{
						if (hull.edges[k].a == closestPoints[j])
						{
							newFace.cIndex = FindIndex(hull.edges[k].b);
							hull.edges.RemoveAt(k);
						}
						else if (hull.edges[k].b == closestPoints[j])
						{
							newFace.cIndex = FindIndex(hull.edges[k].a);
							hull.edges.RemoveAt(k);
						}
					}
				}

				if (!faces.Contains(newFace) && newFace.cIndex != -1 && newFace.bIndex != -1 && newFace.aIndex != -1) faces.Add(newFace);
			}
		}
	}

	
	return faces;
}

ConvexHull UMeshGenerator::ConstructHull(const TArray<FVector>& points) // incomplete quick hull algorithm, will create a hull from four extremal points then assume the rest of the points are inside the hull
{
	// find extremal points

	TArray<FVector> setOfPoints = points;

	FVector highestPoint, lowestPoint, leftMostPoint, rightMostPoint;
	int32 removeIndex;

	// find highest point

	highestPoint = setOfPoints[0];
	removeIndex = 0;

	for (int i = 0; i < setOfPoints.Num(); i++)
	{
		if (setOfPoints[i].Y > highestPoint.Y)
		{
			highestPoint = setOfPoints[i];
			removeIndex = i;
		}
	}

	setOfPoints.RemoveAt(removeIndex);

	// find lowest point

	lowestPoint = setOfPoints[0];
	removeIndex = 0;

	for (int i = 0; i < setOfPoints.Num(); i++)
	{
		if (setOfPoints[i].Y < lowestPoint.Y)
		{
			lowestPoint = setOfPoints[i];
			removeIndex = i;
		}
	}

	setOfPoints.RemoveAt(removeIndex);

	// find left most point

	leftMostPoint = setOfPoints[0];
	removeIndex = 0;

	for (int i = 0; i < setOfPoints.Num(); i++)
	{
		if (setOfPoints[i].X < leftMostPoint.X)
		{
			leftMostPoint = setOfPoints[i];
			removeIndex = i;
		}
	}

	setOfPoints.RemoveAt(removeIndex);

	// find left most point

	rightMostPoint = setOfPoints[0];
	removeIndex = 0;

	for (int i = 0; i < setOfPoints.Num(); i++)
	{
		if (setOfPoints[i].X > rightMostPoint.X)
		{
			rightMostPoint = setOfPoints[i];
			removeIndex = i;
		}
	}

	setOfPoints.RemoveAt(removeIndex);

	ConvexHull newHull;

	newHull.points.Add(highestPoint);
	newHull.points.Add(lowestPoint);
	newHull.points.Add(leftMostPoint);
	newHull.points.Add(rightMostPoint);
	
	for (int i = 0; i < setOfPoints.Num(); i++)
	{
		// add rest of points to hull info
		newHull.points.Add(setOfPoints[i]);
	}

	Edge first, second, third, fourth;

	first.a = highestPoint;
	first.b = leftMostPoint;

	second.a = leftMostPoint;
	second.b = lowestPoint;

	third.a = lowestPoint;
	third.b = rightMostPoint;

	fourth.a = rightMostPoint;
	fourth.b = highestPoint;

	newHull.edges.Add(first);
	newHull.edges.Add(second);
	newHull.edges.Add(third);
	newHull.edges.Add(fourth);

	return newHull;

}

int32 UMeshGenerator::FindIndex(FVector vector)
{
	// find the index from set of points that relates to vector parameter, if this point doesn't exist throw error, return -1
	for (int i = 0; i < m_Mesh.Num(); i++)
	{
		if (m_Mesh[i].Equals(vector))
		{
			return i;
		}
	}
	return -1;
}

// Called every frame
void UMeshGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

