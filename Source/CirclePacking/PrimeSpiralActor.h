#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PrimeSpiralActor.generated.h"

UCLASS()
class CIRCLEPACKING_API APrimeSpiralActor : public AActor
{
    GENERATED_BODY()

public:
    APrimeSpiralActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, Category = "Ulam Spiral")
    int32 MaxPrimeCount = 10000;

    UPROPERTY(EditAnywhere, Category = "Ulam Spiral")
    float Spacing = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Ulam Spiral")
    UStaticMesh* PrimeMeshAsset;

    UPROPERTY(EditAnywhere, Category = "Ulam Spiral")
    UMaterialInterface* PrimeMaterial;

private:
    UPROPERTY(EditAnywhere)
    UInstancedStaticMeshComponent* ISMComponent;

    int32 CurrentIndex = 1;
    FVector LastPrimeLocation = FVector::ZeroVector;
    bool bHasFirstPrime = false;

    bool IsPrime(int32 Number) const;
    FVector2D GetUlamSpiralPosition(int32 Index) const;
};
