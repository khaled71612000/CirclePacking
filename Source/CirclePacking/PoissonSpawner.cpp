#include "PoissonSpawner.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

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
    CellSize = Radius / FMath::Sqrt(2.0f);

    const float SeedSpacing = Radius * 2.f;
    const int32 SeedRange = 3;

    for (int32 dx = -SeedRange; dx <= SeedRange; ++dx)
    {
        for (int32 dy = -SeedRange; dy <= SeedRange; ++dy)
        {
            FVector2D Seed = WorldCenter + FVector2D(dx * SeedSpacing, dy * SeedSpacing);
            AddSample(Seed);
            ActiveList.Add(Seed);
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
    FVector Location(Point.X, Point.Y, GetActorLocation().Z);
    FTransform Transform(Location);

    if (MeshOptions.Num() == 0) return;
    UStaticMesh* RandomMesh = MeshOptions[FMath::RandRange(0, MeshOptions.Num() - 1)];
    UInstancedStaticMeshComponent* TargetInstancer = MeshToInstancer.FindRef(RandomMesh);
    if (!TargetInstancer) return;

    int32 Index = TargetInstancer->AddInstance(Transform);

    float Noise = FMath::PerlinNoise2D(Point * 0.001f);
    FLinearColor RandColor = FLinearColor::MakeFromHSV8(Noise * 255, 255, 255);

    TargetInstancer->SetCustomDataValue(Index, 0, RandColor.R, false);
    TargetInstancer->SetCustomDataValue(Index, 1, RandColor.G, false);
    TargetInstancer->SetCustomDataValue(Index, 2, RandColor.B, false);

    Samples.Add(Point);
}

bool APoissonSpawner::IsInNeighborhood(const FVector2D& Point)
{
    FIntPoint GridPos(Point.X / CellSize, Point.Y / CellSize);
    for (int32 dx = -2; dx <= 2; ++dx)
    {
        for (int32 dy = -2; dy <= 2; ++dy)
        {
            FIntPoint NeighborKey = GridPos + FIntPoint(dx, dy);
            if (FVector2D* Neighbor = Grid.Find(NeighborKey))
            {
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

    int32 Index = FMath::RandRange(0, ActiveList.Num() - 1);
    FVector2D Center = ActiveList[Index];

    bool bFound = false;
    for (int32 i = 0; i < K; ++i)
    {
        float Angle = FMath::RandRange(0.f, 2 * PI);
        float R = FMath::FRandRange(Radius, 2 * Radius);
        FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));
        FVector2D Candidate = Center + Dir * R;

        if (FVector2D::Distance(Candidate, WorldCenter) > ChunkSize) continue;
        if (IsInNeighborhood(Candidate)) continue;

        AddSample(Candidate);
        ActiveList.Add(Candidate);
        Grid.Add(FIntPoint(Candidate.X / CellSize, Candidate.Y / CellSize), Candidate);
        bFound = true;
        break;
    }

    if (!bFound)
    {
        ActiveList.RemoveAt(Index);
    }
}
