﻿#include "DLAClusterActor.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Async/ParallelFor.h" // Add this at the top

ADLAClusterActor::ADLAClusterActor()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded()) MeshComponent->SetStaticMesh(CubeMesh.Object);
}

void ADLAClusterActor::BeginPlay()
{
    Super::BeginPlay();

    // One cube starts in the center — this is the seed.
    FIntVector Seed = FIntVector::ZeroValue;
    AggregatePoints.Add(Seed);
    AddInstanceToMesh(Seed);

    // Spawn walkers
    for (int32 i = 0; i < MaxWalkers; ++i)
    {
        Walkers.Add(FWalker(GetRandomEdgePosition()));
    }

    FVector Center = GetActorLocation();
    FVector Extent = FVector(Bounds) * GridSpacing;
    //DrawDebugBox(GetWorld(), Center, Extent, FColor::Green, true, -1, 0, 5.0f); // Thick green box

    if (UStaticMesh* Mesh = MeshComponent->GetStaticMesh())
    {
        FBox Box = Mesh->GetBoundingBox();
        float Diagonal = Box.GetSize().Size(); 

        GridSpacing = Diagonal * 0.28f;
    }
}

void ADLAClusterActor::Tick(float DeltaTime)
{

    //Calls SimulateStep() to move each walker one step.
    Super::Tick(DeltaTime);

    for (const FWalker& Walker : Walkers)
    {
        FVector WorldPos = GetActorLocation() + FVector(Walker.Position) * GridSpacing;
        //DrawDebugPoint(GetWorld(), WorldPos, 10.0f, FColor::Red, false, -1.0f, 0);
    }

	TimeAccumulator += DeltaTime;
	if (TimeAccumulator < SimulationStepRate)
		return;

	TimeAccumulator = 0.f;

    SimulateStep();

    TArray<int32> Completed;

    for (auto& Pair : GrowingInstances)
    {
        int32 Index = Pair.Key;
        float& Time = Pair.Value;
        Time += DeltaTime;

        float Alpha = FMath::Clamp(Time / GrowthDuration, 0.f, 1.f);
        FVector Scale = FMath::Lerp(FVector::ZeroVector, FVector(1.f), Alpha);

        FTransform InstanceTransform;
        if (MeshComponent->GetInstanceTransform(Index, InstanceTransform, true))
        {
            InstanceTransform.SetScale3D(Scale);
            MeshComponent->UpdateInstanceTransform(Index, InstanceTransform, true, true, true);
        }

        if (Alpha >= 1.f)
        {
            Completed.Add(Index);
        }
    }

    for (int32 Index : Completed)
    {
        GrowingInstances.Remove(Index);
    }

}

void ADLAClusterActor::SimulateStep()
{
    // Gradually reduce the number of active walkers over time to simulate slowing coral growth.
    // For every 20 simulation steps, reduce the walker count by 1.
    // Clamp to a minimum of 5 walkers to prevent growth from stalling completely.
    ++StepCount;
    int32 TargetWalkerCount = FMath::Max(5, MaxWalkers - StepCount / 90);
    if (Walkers.Num() > TargetWalkerCount)
    {
        //No new walkers are added anymore, the pool just shrinks as the sim matures.
        Walkers.SetNum(TargetWalkerCount);
    }

    // Store positions to add to the crystal later
    TArray<FIntVector> PointsToAdd;

    // New walker array that we’ll fill during simulation
    TArray<FWalker> UpdatedWalkers;
    UpdatedWalkers.SetNumUninitialized(Walkers.Num());

    // Protect shared access to AggregatePoints and PointsToAdd
    FCriticalSection Mutex;

    ParallelFor(Walkers.Num(), [&](int32 i)
        {
            FWalker Walker = Walkers[i];

            // Determine direction toward the center on each axis (X, Y, Z)
            // If walker is positive on an axis, move -1 (toward 0); if negative, move +1
            // If already at 0 on that axis, don't move (set to 0)
            //FIntVector DirectionToCenter(
            //    Walker.Position.X == 0 ? 0 : (Walker.Position.X > 0 ? -1 : 1),
            //    Walker.Position.Y == 0 ? 0 : (Walker.Position.Y > 0 ? -1 : 1),
            //    Walker.Position.Z == 0 ? 0 : (Walker.Position.Z > 0 ? -1 : 1)
            //);

            //// Choose a random axis (0 = X, 1 = Y, 2 = Z) to move on
            //int32 Axis = FMath::RandRange(0, 2);
            //int32 Step = DirectionToCenter[Axis];

            // Get the directional step on that axis (inward)
            // If already centered on that axis (step = 0), move randomly to prevent getting stuck
            //if (Step == 0)
            //    Step = FMath::RandBool() ? 1 : -1;

            //Walker.Position[Axis] += Step;

            // --- Pure 3D Random Walk (Brownian Motion) ---
			Walker.Position.X += FMath::RandRange(-1, 1);
			Walker.Position.Y += FMath::RandRange(-1, 1);
			Walker.Position.Z += FMath::RandRange(-1, 1);


            // Check if this walker is adjacent to any point in the aggregate
            if (IsAdjacentToAggregate(Walker.Position))
            {
                // Lock and record this position for aggregation and mesh spawning
                Mutex.Lock();
                PointsToAdd.Add(Walker.Position);
                Mutex.Unlock();

                // Respawn walker at edge to keep constant walker count
                Walker = FWalker(GetRandomEdgePosition());
            }
            else
            {
                // If the walker's position exceeds the allowed grid boundary, respawn it
                if (FMath::Abs(Walker.Position.X) > Bounds ||
                    FMath::Abs(Walker.Position.Y) > Bounds ||
                    FMath::Abs(Walker.Position.Z) > Bounds)
                {
                    Walker = FWalker(GetRandomEdgePosition());
                }
            }

            UpdatedWalkers[i] = Walker;
        });

    // Apply all recorded aggregation results on main thread
    for (const FIntVector& Pos : PointsToAdd)
    {
        if (!AggregatePoints.Contains(Pos))
        {
            AggregatePoints.Add(Pos);
            AddInstanceToMesh(Pos);
        }
    }

    // Replace walkers with the updated set
    Walkers = MoveTemp(UpdatedWalkers);
}

bool ADLAClusterActor::IsAdjacentToAggregate(const FIntVector& Pos) const
{
    //Old 6-direction check (axis only):
    //static const TArray<FIntVector> Offsets = {
    //    {1,0,0}, {-1,0,0},
    //    {0,1,0}, {0,-1,0},
    //    {0,0,1}, {0,0,-1}
    //};
    //New 26-direction check (all neighbors around a voxel):
    // Checks if the given position is adjacent (in any direction) to an already aggregated crystal point.
    // This includes all 26 neighboring positions in a 3x3x3 cube around the current voxel.
    for (int32 X = -1; X <= 1; ++X)
    {
        for (int32 Y = -1; Y <= 1; ++Y)
        {
            for (int32 Z = -1; Z <= 1; ++Z)
            {
                if (X == 0 && Y == 0 && Z == 0)
                    continue; // Skip self

                FIntVector Neighbor = Pos + FIntVector(X, Y, Z);
                if (AggregatePoints.Contains(Neighbor))
                    return true;
            }
        }
    }

    // No adjacent aggregated neighbor found
    return false;
}

FIntVector ADLAClusterActor::GetRandomEdgePosition() const
{
    FIntVector Pos;
    int32 Axis = FMath::RandRange(0, 2);
    int32 Side = FMath::RandBool() ? Bounds : -Bounds;

    for (int32 i = 0; i < 3; ++i)
    {
		/*  Set the chosen axis to the edge(+Bounds or -Bounds).
			Randomize the other two.*/
        Pos[i] = (i == Axis) ? Side : FMath::RandRange(-Bounds, Bounds);
    }

    return Pos;
}

void ADLAClusterActor::AddInstanceToMesh(const FIntVector& Pos)
{
    FVector Location = FVector(Pos) * GridSpacing;

    FRotator RandomRot = FRotator(
        FMath::RandRange(0.f, 360.f), // Pitch
        FMath::RandRange(0.f, 360.f), // Yaw
        FMath::RandRange(0.f, 360.f)  // Roll
    );

    UStaticMesh* Mesh = MeshComponent->GetStaticMesh();
    float HalfHeight = 50.0f;
    if (Mesh)
    {
        FBox Box = Mesh->GetBoundingBox();
        HalfHeight = Box.GetExtent().Z;
    }

    int32 InstanceIndex = MeshComponent->AddInstance(FTransform(RandomRot, Location, FVector::ZeroVector));
    GrowingInstances.Add(InstanceIndex, 0.0f);

    //FVector MeshCenterOffset(0, 0, HalfHeight);
    //FVector RotatedOffset = RandomRot.RotateVector(MeshCenterOffset);
    //FVector WorldPos = MeshComponent->GetComponentTransform().TransformPosition(Location + RotatedOffset);

	//DrawDebugSphere(GetWorld(), WorldPos, 45.0f, 12, FColor::Yellow, false, 1.0f);
}
