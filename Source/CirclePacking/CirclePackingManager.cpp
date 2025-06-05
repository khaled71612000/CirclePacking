#include "CirclePackingManager.h"

ACirclePackingManager::ACirclePackingManager()
{
	PrimaryActorTick.bCanEverTick = true;
	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	RootComponent = InstancedMesh;
}

void ACirclePackingManager::BeginPlay()
{
	Super::BeginPlay();

    if (CircleMesh)
        InstancedMesh->SetStaticMesh(CircleMesh);

    InstancedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InstancedMesh->SetVisibility(true);
    InstancedMesh->NumCustomDataFloats = 4;
}

void ACirclePackingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //TimeAccumulator += DeltaTime;
    //if (TimeAccumulator < SimulationStepRate)
    //    return;

    //TimeAccumulator = 0.f;

    TrySpawnNewCircle();

    InstancedMesh->ClearInstances();

    for (int32 i = 0; i < Circles.Num(); ++i)
    {
        auto& Circle = Circles[i];

        // Grow
        if (Circle.Radius < Circle.TargetRadius)
            Circle.Radius = FMath::Min(Circle.Radius + Circle.GrowthRate * SimulationStepRate, Circle.TargetRadius);

        // Age
        Circle.Age += SimulationStepRate;

        // Transform
        FVector Loc = FVector(Circle.Position.X, Circle.Position.Y, Circle.Radius * 0.02f);
        FVector Scale = FVector(Circle.Radius / 50.f, Circle.Radius / 50.f, 0.05f);
        //FRotator Rot(0.f, 0.f, Circle.Age * 30.f);
        FTransform InstanceTransform(FRotator::ZeroRotator, Loc, Scale);
        int32 Index = InstancedMesh->AddInstance(InstanceTransform);

        float Emissive = 0.f;

        if (Circle.Radius < Circle.TargetRadius)
        {
            // Growing → ramp up
            Emissive = FMath::Clamp(Circle.Radius / Circle.TargetRadius * 0.6f, 0.f, 0.6f);
        }
        else
        {
            // Fully grown → fade over 5 seconds
            Emissive = FMath::Clamp((1.f - (Circle.Age - (Circle.TargetRadius / Circle.GrowthRate)) / 5.f) * 0.6f, 0.f, 0.6f);
        }

        TArray<float> CustomData = {
            Circle.Color.R,
            Circle.Color.G,
            Circle.Color.B,
            Emissive
        };

        InstancedMesh->SetCustomData(Index, CustomData);
    }
}

bool ACirclePackingManager::IsOverlapping(const FVector2D& Pos, float Radius) const
{
for (const auto& Other : Circles)
    {
        float DistSq = FVector2D::DistSquared(Other.Position, Pos);
        float MinDist = Radius + Other.TargetRadius;
        if (DistSq < MinDist * MinDist)
            return true;
    }
    return false;
}

void ACirclePackingManager::TrySpawnNewCircle()
{
    const int32 MaxAttempts = 500;
    for (int i = 0; i < MaxAttempts; ++i)
    {
        FVector2D TryPos = FVector2D(
            FMath::FRandRange(-CanvasSize, CanvasSize),
            FMath::FRandRange(-CanvasSize, CanvasSize)
        );

        float ExponentBias = 0.01f; // Lower = more tiny, rarer big
        float Alpha = FMath::FRand();
        float fRandRange = MinTargetRadius + -FMath::Loge(1.f - Alpha) / ExponentBias;
        fRandRange = FMath::Clamp(fRandRange, MinTargetRadius, MaxTargetRadius);

        if (!IsOverlapping(TryPos, fRandRange))
        {
            FCircleData NewCircle;
            NewCircle.Position = TryPos;
            NewCircle.TargetRadius = fRandRange;
            NewCircle.ID = Circles.Num();
            NewCircle.Color = FLinearColor::MakeRandomColor();
            Circles.Add(NewCircle);
            break;
        }
    }
}