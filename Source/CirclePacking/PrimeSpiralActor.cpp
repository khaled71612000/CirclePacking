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

        // === DEBUG VISUALIZATION OF SPIRAL ===
        FVector DebugWorldPos = FVector(GridPos.X * Spacing, GridPos.Y * Spacing, 50);
        DrawDebugPoint(GetWorld(), GetActorLocation() + DebugWorldPos, 10.f, FColor::Green, false, 5.f);
        DrawDebugString(GetWorld(), GetActorLocation() + DebugWorldPos + FVector(0, 0, 25), FString::FromInt(CurrentIndex), nullptr, FColor::White, 5.f, false);


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
    // If Number is divisible by anything bigger than its square root, it would have already been caught by a smaller factor
    for (int32 i = 2; i * i <= Number; ++i)
    {
        if (Number % i == 0) return false;
    }
    return true;
}

//17 16 15 14 13
//18  5  4  3 12
//19  6  1  2 11
//20  7  8  9 10
//21 22 23 24 25

//Index 1 = center →(0, 0)
//
//Index 2 = right →(1, 0)
//
//Index 3 = up →(1, 1)
//
//Index 4 = left →(0, 1)
//
//Index 5 = left again →(−1, 1)
//
//Index 6 = down →(−1, 0)
//
//Index 7 = down again →(−1, −1)
//
//Index 8 = right →(0, −1)
//
//Index 9 = right →(1, −1)


FVector2D APrimeSpiralActor::GetUlamSpiralPosition(int32 Index) const
{
    //Imagine drawing numbers starting at the center, then going right, up, left, down in a square spiral.
    //We start at the center: (0, 0)
    int32 x = 0, y = 0;
    //First direction: move right (dx=1, dy=0)
    int32 dx = 1, dy = 0;
   //We move 1 step before turning, and this indicates the steps we take before turning. Every 2 direction changes we increase segment length
    int32 segment_length = 1;
   //Tracking how many steps and segments we’ve done.
    int32 segment_passed = 0;
    int32 steps_in_segment = 0;

    for (int32 i = 1; i < Index; ++i)
    {
        //Go from 1 to the target index, one step at a time
        x += dx;
        y += dy;
        steps_in_segment++;
        //If we've taken enough steps in this direction
        if (steps_in_segment == segment_length)
        {
            //Reset step counter.
            steps_in_segment = 0;
            segment_passed++;

            //Turn 90° clockwise.
            int32 temp = dx;
            dx = -dy;
            dy = temp;

            //Every second turn, we make the next segment longer
            if (segment_passed % 2 == 0)
            {
                segment_length++;
            }
        }
    }

    return FVector2D((float)x, (float)y);
}
