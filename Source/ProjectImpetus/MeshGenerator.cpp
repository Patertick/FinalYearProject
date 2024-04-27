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


}

Mesh UMeshGenerator::ConstructMesh(const TArray<FVector>& points)
{
	Mesh newMesh;
	for (int i = 0; i < points.Num(); i += 3)
	{
		Triangle newTriangle;

		newTriangle.a = points[i + 2];
		newTriangle.b = points[i + 1];
		newTriangle.c = points[i];

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
		for (const Triangle& other : mesh.triangles)
		{

			if (other.a.Equals(tri.a) || other.a.Equals(tri.b) || other.a.Equals(tri.c))
			{
				if (other.b.Equals(tri.a) || other.b.Equals(tri.b) || other.b.Equals(tri.c))
				{
					numberOfTrianglesThatSharesTwoPoints++;
				}
				else if (other.c.Equals(tri.a) || other.c.Equals(tri.b) || other.c.Equals(tri.c))
				{
					numberOfTrianglesThatSharesTwoPoints++;
				}
			}
			else if (other.c.Equals(tri.a) || other.c.Equals(tri.b) || other.c.Equals(tri.c))
			{
				if (other.b.Equals(tri.a) || other.b.Equals(tri.b) || other.b.Equals(tri.c))
				{
					numberOfTrianglesThatSharesTwoPoints++;
				}
				else if (other.a.Equals(tri.a) || other.a.Equals(tri.b) || other.a.Equals(tri.c))
				{
					numberOfTrianglesThatSharesTwoPoints++;
				}
			}
			else if (other.b.Equals(tri.a) || other.b.Equals(tri.b) || other.b.Equals(tri.c))
			{
				if (other.a.Equals(tri.a) || other.a.Equals(tri.b) || other.a.Equals(tri.c))
				{
					numberOfTrianglesThatSharesTwoPoints++;
				}
				else if (other.c.Equals(tri.a) || other.c.Equals(tri.b) || other.c.Equals(tri.c))
				{
					numberOfTrianglesThatSharesTwoPoints++;
				}
			}
		}

		// if this triangle doesn't share two points with 6 other triangles, it must be disjoint
		if (numberOfTrianglesThatSharesTwoPoints < 3)
		{
			return false;
		}
	}

	return true;
}


Triangle UMeshGenerator::GenerateTriangle(FVector direction, FVector firstPoint, FVector secondPoint)
{
	Triangle newTri;
	if (firstPoint != FVector{})
	{
		newTri.a = firstPoint;
	}
	else
	{
		float randDist = FMath::FRandRange(10.0f, 100.0f);
		do {
			newTri.a = FVector{ FMath::FRandRange(-100.0f, 100.0f), FMath::FRandRange(-100.0f, 100.0f), FMath::FRandRange(-100.0f, 100.0f) };
		} while (FVector::Distance(newTri.a, m_Centroid) < randDist);
	}
	if (secondPoint != FVector{})
	{
		newTri.b = secondPoint;
	}
	else
	{
		float randDist = FMath::FRandRange(10.0f, 100.0f);
		do {
			newTri.b = FVector{ FMath::FRandRange(-100.0f, 100.0f), FMath::FRandRange(-100.0f, 100.0f), FMath::FRandRange(-100.0f, 100.0f) };
		} while (FVector::Distance(newTri.b, m_Centroid) < randDist);
		//newTri.b = FVector{ FMath::FRandRange(-100.0f, 100.0f), FMath::FRandRange(-100.0f, 100.0f), FMath::FRandRange(-100.0f, 100.0f) };
	}
	newTri.c.X = ((newTri.a.X + newTri.b.X) / 2) + direction.GetSafeNormal().X * FMath::FRandRange(-100.0f, 100.0f);
	newTri.c.Y = ((newTri.a.Y + newTri.b.Y) / 2) + direction.GetSafeNormal().Y * FMath::FRandRange(-100.0f, 100.0f);
	newTri.c.Z = ((newTri.a.Z + newTri.b.Z) / 2) + direction.GetSafeNormal().Z * FMath::FRandRange(-100.0f, 100.0f);

	return newTri;

}

// Called when the game starts
void UMeshGenerator::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


void UMeshGenerator::CreateNewMesh(UStaticMeshComponent* mesh, FStaticMaterial mat)
{

	// GENERATE 3D CUBE MAP USING BOOLEAN VALUES

	m_XBounds = FMath::RandRange(10, 25);
	m_YBounds = FMath::RandRange(10, 25);
	m_ZBounds = FMath::RandRange(25, 50);

	for (int z = 0; z < m_ZBounds; z++)
	{
		Row newRow;
		for (int y = 0; y < m_YBounds; y++)
		{
			Column newCol;
			for (int x = 0; x < m_XBounds; x++)
			{
				newCol.col.Add(false);
			}
			newRow.row.Add(newCol);
		}
		m_MeshMap.Add(newRow);
	}

	int32 numberOfCubes = FMath::RandRange((m_XBounds * m_YBounds * m_ZBounds) / 10, (m_XBounds * m_YBounds * m_ZBounds) / 5);

	int32 randomX, randomY, randomZ;

	m_MeshMap[m_ZBounds / 2].row[m_YBounds / 2].col[m_XBounds / 2] = true;

	for (int i = 0; i < numberOfCubes - 1; i++)
	{
		bool validLocation{ true };
		do {
			randomX = FMath::RandRange(0, m_XBounds - 1);
			randomY = FMath::RandRange(0, m_YBounds - 1);
			randomZ = FMath::RandRange(0, m_ZBounds - 1);

			validLocation = false;
			if (m_MeshMap[randomZ].row[randomY].col[randomX])
			{
				// do nothing
			}
			else
			{
				Index randomIndex;
				randomIndex.x = randomX;
				randomIndex.y = randomY;
				randomIndex.z = randomZ;
				TArray<Index> adjacentCubes = GetAdjacentItems(randomIndex);

				for (Index value : adjacentCubes)
				{
					if (value.x < 0 || value.x >= m_XBounds || value.y < 0 || value.y >= m_YBounds || value.z < 0 || value.z >= m_ZBounds)
					{
						// not valid
					}
					else
					{
						if (m_MeshMap[value.z].row[value.y].col[value.x])
						{
							validLocation = true; // adjacent cube exists
						}
					}
				}
			}
		} while (!validLocation);

		m_MeshMap[randomZ].row[randomY].col[randomX] = true;
	}

	// REMOVE UNNECESSARY CUBES

	for (int z = 0; z < m_MeshMap.Num(); z++)
	{
		for (int y = 0; y < m_MeshMap[z].row.Num(); y++)
		{
			for (int x = 0; x < m_MeshMap[z].row[y].col.Num(); x++)
			{
				if (m_MeshMap[z].row[y].col[x])
				{
					int32 numberOfAdjacentCubes{ 0 };
					Index currentIndex;
					currentIndex.x = x;
					currentIndex.y = y;
					currentIndex.z = z;
					TArray<Index> adjacentCubes = GetAdjacentItems(currentIndex);
					for (Index value : adjacentCubes)
					{
						if (value.x < 0 || value.x >= m_XBounds || value.y < 0 || value.y >= m_YBounds || value.z < 0 || value.z >= m_ZBounds)
						{
							// not valid
						}
						else
						{
							if (m_MeshMap[value.z].row[value.y].col[value.x])
							{
								numberOfAdjacentCubes++;
							}
						}
					}

					if (numberOfAdjacentCubes >= adjacentCubes.Num())
					{
						m_MeshMap[z].row[y].col[x] = false;
					}

				}
			}
		}
	}

	// TRANSLATE INTO MESH

	FVector offset = FVector{ static_cast<float>(m_XBounds * 2), static_cast<float>(m_YBounds * 2), static_cast<float>(m_ZBounds * 2) };

	for (int z = 0; z < m_MeshMap.Num(); z++)
	{
		for (int y = 0; y < m_MeshMap[z].row.Num(); y++)
		{
			for (int x = 0; x < m_MeshMap[z].row[y].col.Num(); x++)
			{
				if (m_MeshMap[z].row[y].col[x])
				{
					TArray<FVector> cubeIndices = GenerateCube(FVector{ x * KSIZEOFCUBE, y * KSIZEOFCUBE, z * KSIZEOFCUBE }, offset);
					for (FVector loc : cubeIndices)
					{
						m_Mesh.Add(loc);
					}
				}
			}
		}
	}

	//m_Mesh = GenerateCube(FVector{ 0.0f, 0.0f, 0.0f });

	m_Centroid = FVector{ 0.0f, 0.0f, 0.0f };

	for (FVector point : m_Mesh)
	{
		m_Centroid += point;
	}

	m_Centroid /= m_Mesh.Num();

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
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, m_Mesh[i].ToString());
		vertexIDs[i] = meshDescBuilder.AppendVertex(m_Mesh[i]);
	}

	// array that stores vertex instances (3 vertices per face)
	TArray<FVertexInstanceID> vertexInsts;

	// using faces as reference, assign vertex instances
	for (int i = 0; i < vertexIDs.Num(); i += 3)
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
	staticMesh->GetStaticMaterials().Add(mat);

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

Index UMeshGenerator::GetRandomCube()
{
	// get a random cube that isn't index

	TArray<Index> foundCubes;

	for (int z = 0; z < m_MeshMap.Num(); z++)
	{
		for (int y = 0; y < m_MeshMap[z].row.Num(); y++)
		{
			for (int x = 0; x < m_MeshMap[z].row[y].col.Num(); x++)
			{
				if (m_MeshMap[z].row[y].col[x])
				{
					Index foundCube;
					foundCube.x = x;
					foundCube.y = y;
					foundCube.z = z;
					foundCubes.Add(foundCube);
				}
			}
		}
	}

	if (foundCubes.Num() <= 0)
	{
		Index invalidIndex;
		invalidIndex.x = -1;
		return invalidIndex;
	}
	else
	{
		return foundCubes[FMath::RandRange(0, foundCubes.Num() - 1)];
	}
}

bool UMeshGenerator::IsIndexConnected(Index start, Index end)
{
	TArray<Index> openList;
	TArray<Index> closedList;

	openList.Add(start);

	while (openList.Num() > 0)
	{
		Index closestIndex;
		closestIndex.x = -1;

		for (Index item : openList)
		{
			if (closestIndex.x < 0)
			{
				closestIndex = item;
			}
			else
			{
				int32 oldManhattanDistance = abs(closestIndex.x - end.x) + abs(closestIndex.y - end.y) + abs(closestIndex.z - end.z);
				int32 newManhattanDistance = abs(item.x - end.x) + abs(item.y - end.y) + abs(item.z - end.z);
				if (newManhattanDistance < oldManhattanDistance)
				{
					closestIndex = item;
				}
			}
		}

		if (closestIndex == end)
		{
			return true;
		}

		closedList.Add(closestIndex);

		TArray<Index> adjacentElements = GetAdjacentItems(closestIndex);
		for (Index index : adjacentElements)
		{
			if (index.x < 0 || index.x >= m_XBounds || index.y < 0 || index.y >= m_YBounds || index.z < 0 || index.z >= m_ZBounds)
			{
				// invalid
			}
			else
			{
				if (m_MeshMap[index.z].row[index.y].col[index.x])
				{
					openList.Add(index);
				}
			}
		}
	}

	return false;
}

TArray<Index> UMeshGenerator::GetAdjacentItems(Index index)
{
	TArray<Index> connectedIndices;

	Index connectedIndex;

	connectedIndex.x = index.x - 1;
	connectedIndex.y = index.y;
	connectedIndex.z = index.z;
	connectedIndices.Add(connectedIndex);

	connectedIndex.x = index.x + 1;
	connectedIndex.y = index.y;
	connectedIndex.z = index.z;
	connectedIndices.Add(connectedIndex);

	connectedIndex.x = index.x;
	connectedIndex.y = index.y - 1;
	connectedIndex.z = index.z;
	connectedIndices.Add(connectedIndex);

	connectedIndex.x = index.x;
	connectedIndex.y = index.y + 1;
	connectedIndex.z = index.z;
	connectedIndices.Add(connectedIndex);

	connectedIndex.x = index.x;
	connectedIndex.y = index.y;
	connectedIndex.z = index.z - 1;
	connectedIndices.Add(connectedIndex);

	connectedIndex.x = index.x;
	connectedIndex.y = index.y;
	connectedIndex.z = index.z + 1;
	connectedIndices.Add(connectedIndex);

	return connectedIndices;
}

TArray<FVector> UMeshGenerator::GenerateCube(FVector centrePoint, FVector offset)
{
	TArray<FVector> points;

	// FRONT

	// triangle 1

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // top right

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // bottom right

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // bottom left

	// triangle 2

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // bottom left

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // top left

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // top right

	// BACK

	// triangle 1

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // bottom left

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // bottom right

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // top right

	// triangle 2

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // top right

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // top left

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // bottom left

	// TOP

	// triangle 1

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // back right

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // forward right

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // forward left

	// triangle 2

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // forward left

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // back left

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // back right

	// BOTTOM

	// triangle 1

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // forward left

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // forward right

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // back right

	// triangle 2

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // back right

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // back left

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // forward left

	// LEFT

	// triangle 1

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // back bottom

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // forward top

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // forward bottom 

	// triangle 2

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // back bottom

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // back top

	points.Add(FVector{ centrePoint.X - (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // forward top

	// RIGHT

	// triangle 1

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // forward bottom 

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // forward top

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // back bottom

	// triangle 2

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y + (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // forward top

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z + (KSIZEOFCUBE / 2) } - offset); // back top

	points.Add(FVector{ centrePoint.X + (KSIZEOFCUBE / 2), centrePoint.Y - (KSIZEOFCUBE / 2), centrePoint.Z - (KSIZEOFCUBE / 2) } - offset); // back bottom

	return points;

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

