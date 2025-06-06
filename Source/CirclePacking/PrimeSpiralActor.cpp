#include "PrimeSpiralActor.h"
#include "DrawDebugHelpers.h"

APrimeSpiralActor::APrimeSpiralActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ISMComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
    RootComponent = ISMComponent;
}

void APrimeSpiralActor::BeginPlay()
{
    Super::BeginPlay();

    if (PrimeMeshAsset)
    {
        ISMComponent->SetStaticMesh(PrimeMeshAsset);
        if (PrimeMaterial)
        {
            ISMComponent->SetMaterial(0, PrimeMaterial);
        }


		FVector MeshExtent = PrimeMeshAsset->GetBounds().BoxExtent;
		float SafeSpacing = FMath::Max3(MeshExtent.X, MeshExtent.Y, MeshExtent.Z) * 2.0f;

		Spacing = SafeSpacing;
    }
}

void APrimeSpiralActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentIndex > MaxPrimeCount || !PrimeMeshAsset)
        return;

    if (IsPrime(CurrentIndex))
    {
        FVector2D GridPos = GetUlamSpiralPosition(CurrentIndex);
        float ZOffset = FMath::Sin(CurrentIndex * 0.1f) * 20.0f;
		FVector WorldPos(GridPos.X * Spacing, GridPos.Y * Spacing, 0);

        // Spawn instance at prime position
        //FRotator PrimeRotation = FRotator(0, CurrentIndex % 360, 0);
        FVector Scale = FVector(1.0f + FMath::Sin(CurrentIndex * 0.1f) * 0.5f);
        FTransform InstanceTransform(FRotator::ZeroRotator, WorldPos);
        InstanceTransform.SetScale3D(Scale * 0.5);
        ISMComponent->AddInstance(InstanceTransform);

        //// Draw line from last prime to this one
		if (bHasFirstPrime)
		{
			FLinearColor Color = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Blue, CurrentIndex / (float)MaxPrimeCount);
			DrawDebugLine(GetWorld(), GetActorLocation() + LastPrimeLocation + FVector(0, 0, 10), GetActorLocation() + WorldPos + FVector(0, 0, 10), Color.ToFColor(true), false, 10.f, 0, 15.0f);
		}

        //// Draw prime number above the shape
        //DrawDebugString(GetWorld(), WorldPos + FVector(0, 0, 100), FString::FromInt(CurrentIndex), nullptr, FColor::Red, 10.f, false, 1.f);

        LastPrimeLocation = WorldPos;
        bHasFirstPrime = true;
    }

    ++CurrentIndex;
}

bool APrimeSpiralActor::IsPrime(int32 Number) const
{
    if (Number < 2) return false;
    for (int32 i = 2; i * i <= Number; ++i)
    {
        if (Number % i == 0) return false;
    }
    return true;
}

FVector2D APrimeSpiralActor::GetUlamSpiralPosition(int32 Index) const
{
    int32 x = 0, y = 0;
    int32 dx = 0, dy = -1;

    for (int32 i = 0; i < Index; ++i)
    {
        if (x == y || (x < 0 && x == -y) || (x > 0 && x == 1 - y))
        {
            int32 temp = dx;
            dx = -dy;
            dy = temp;
        }
        x += dx;
        y += dy;
    }

    return FVector2D(x, y);
}
