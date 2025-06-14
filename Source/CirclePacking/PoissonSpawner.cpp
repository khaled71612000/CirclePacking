#include "PoissonSpawner.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

//Poisson - disc sampling makes natural - looking but non - overlapping distribution.
//Useful for forests, rocks, NPCs, anything that needs space around it.

APoissonSpawner::APoissonSpawner()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void APoissonSpawner::BeginPlay()
{
    Super::BeginPlay();

    for (UStaticMesh* Mesh : MeshOptions)
    {
        if (!Mesh) continue;

        FString Name = Mesh->GetName() + "_Instancer";
        UInstancedStaticMeshComponent* NewInstancer = NewObject<UInstancedStaticMeshComponent>(this, FName(*Name));
        NewInstancer->RegisterComponent();
        NewInstancer->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        NewInstancer->SetStaticMesh(Mesh);

        if (MaterialInstance)
        {
            NewInstancer->SetMaterial(0, MaterialInstance);
            NewInstancer->NumCustomDataFloats = 3;
        }

        MeshToInstancer.Add(Mesh, NewInstancer);
    }

    WorldCenter = FVector2D(GetActorLocation().X, GetActorLocation().Y);
    // Each cell on the 2D grid can store only one point max. To avoid overlap,
    // cell size ≈ radius / √2 ensures points in neighboring cells won't be too close.
    // If we simply use CellSize = Radius, then a point in one cell could still be too close to a point in a diagonal cel
    // Because the diagonal of a square is longer than its side
    //┌─────┬─────┐
    //│  A  │ ?   │
    //├─────┼─────┤
    //│  ?  │ B   │
    //└─────┴─────┘
    // If A and B are in diagonal cells, they’re r = √2 × CellSize apart as thats the square diagonal
    CellSize = Radius / FMath::Sqrt(2.0f);

    //Imagine placing flags on a chessboard around the middle square. Each flag is a potential starting point for new trees.
    // this ensure safe spacing with starting seeds so if radius is 100 it should be 200 uu apart
    const float SeedSpacing = Radius * 2.f;
    const int32 SeedRange = 5;
	/*Seeds a bunch of starting points in a grid near the center.

	These points are added to a list of "active points."

	Each point is also stored in a 2D grid(like putting it on a map).*/

    for (int32 dx = -SeedRange; dx <= SeedRange; ++dx)
    {
        for (int32 dy = -SeedRange; dy <= SeedRange; ++dy)
        {
            FVector2D Seed = WorldCenter + FVector2D(dx * SeedSpacing, dy * SeedSpacing);
            AddSample(Seed);
            ActiveList.Add(Seed);
            //Because we want to quickly and efficiently check nearby points during spawning
            //without scanning the entire list of samples.
            //It converts the real-world coordinates like X = 243, Y = 510
            // Into a grid index like(5, 10) — like finding the square on a chessboard

            FIntPoint GridKey = FIntPoint(Seed.X / CellSize, Seed.Y / CellSize);
            Grid.Add(GridKey, Seed);
        }
    }
}

void APoissonSpawner::Tick(float DeltaTime)
{
    SpawnAccumulator += DeltaTime;
    if (SpawnAccumulator < SpawnInterval) return;
    SpawnAccumulator = 0.f;

    //If we have no active points left :
    //Expand the spawn range(ChunkSize).
    //Pick 5 old points to try again.
    if (ActiveList.Num() == 0)
    {
        ChunkSize += 200.f;

        if (Samples.Num() > 0)
        {
            for (int32 i = 0; i < 5; ++i)
            {
                FVector2D NewSeed = Samples[FMath::RandRange(0, Samples.Num() - 1)];
                ActiveList.Add(NewSeed);
            }
        }
    }

    //Try to spawn a new mesh near a random active point.
    for (int32 i = 0; i < PointsPerTick; ++i)
    {
        GenerateNextPoints();
    }

    for (auto& Pair : MeshToInstancer)
    {
        Pair.Value->MarkRenderStateDirty();
    }
}

void APoissonSpawner::AddSample(const FVector2D& Point)
{
    //Convert the 2D point into a 3D position (X, Y, and actor’s Z).
    FVector Location(Point.X, Point.Y, GetActorLocation().Z);
    FTransform Transform(Location);

    if (MeshOptions.Num() == 0) return;
    UStaticMesh* RandomMesh = MeshOptions[FMath::RandRange(0, MeshOptions.Num() - 1)];
    UInstancedStaticMeshComponent* TargetInstancer = MeshToInstancer.FindRef(RandomMesh);
    if (!TargetInstancer) return;

    int32 Index = TargetInstancer->AddInstance(Transform);

    //Use Perlin noise to generate a smooth, unique color.
    float Noise = FMath::PerlinNoise2D(Point * 0.001f);
    FLinearColor RandColor = FLinearColor::MakeFromHSV8(Noise * 255, 255, 255);

    //Apply that color to the mesh (custom data floats).
    TargetInstancer->SetCustomDataValue(Index, 0, RandColor.R, false);
    TargetInstancer->SetCustomDataValue(Index, 1, RandColor.G, false);
    TargetInstancer->SetCustomDataValue(Index, 2, RandColor.B, false);

    //Add that point to the list of all samples.
    Samples.Add(Point);
}

bool APoissonSpawner::IsInNeighborhood(const FVector2D& Point)
{
    //Converts the 2D point into a cell on a grid (like a square on a chessboard).
    FIntPoint GridPos(Point.X / CellSize, Point.Y / CellSize);
    
    //Looks at the 25 nearby grid cells around it.
    for (int32 dx = -5; dx <= 5; ++dx)
    {
        for (int32 dy = -5; dy <= 5; ++dy)
        {
            FIntPoint NeighborKey = GridPos + FIntPoint(dx, dy);
            if (FVector2D* Neighbor = Grid.Find(NeighborKey))
            {
                //Makes sure new points aren’t too close to any existing ones. It’s too close → skip this candidate.
                if (FVector2D::Distance(*Neighbor, Point) < Radius)
                    return true;
            }
        }
    }
    return false;
}

void APoissonSpawner::GenerateNextPoints()
{
    if (ActiveList.Num() == 0) return;

    //Pick a random point from active list
    int32 Index = FMath::RandRange(0, ActiveList.Num() - 1);
    FVector2D Center = ActiveList[Index];

    bool bFound = false;
    //Try K amount of times to find a new angle and distance and make sure its a good point
    for (int32 i = 0; i < K; ++i)
    {
        float Angle = FMath::RandRange(0.f, 2 * PI);
        //“You can plant a new tree anywhere 1–2 meters from this one.”
        float R = FMath::FRandRange(Radius, 2 * Radius);
        //Turns the random angle into a 2D direction.
        FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));
        //multi dir by distance and add cetner to shift
        FVector2D Candidate = Center + Dir * R;
        //make sure inside spawn area and its not too close to another points
        if (FVector2D::Distance(Candidate, WorldCenter) > ChunkSize) continue;
        if (IsInNeighborhood(Candidate)) continue;

        //spawn it
        AddSample(Candidate);
        ActiveList.Add(Candidate);
        Grid.Add(FIntPoint(Candidate.X / CellSize, Candidate.Y / CellSize), Candidate);
        bFound = true;
        break;
    }

    //Remove the original point from the active list if it fails the distance and chunk size continue
    if (!bFound)
    {
        ActiveList.RemoveAt(Index);
    }
}
