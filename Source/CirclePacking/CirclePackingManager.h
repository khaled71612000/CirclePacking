#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "CirclePackingManager.generated.h"


USTRUCT()
struct FCircleData
{
    GENERATED_BODY()

    FVector2D Position;
    float Radius = 0.f;
    float TargetRadius = 50.f;
    float GrowthRate = 20.f;

    int32 ID = -1;
    float Age = 0.f;
    FLinearColor Color = FLinearColor::White;
};


UCLASS()
class CIRCLEPACKING_API ACirclePackingManager : public AActor
{
	GENERATED_BODY()
	
public:
	ACirclePackingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere)
    UStaticMesh* CircleMesh;

    UPROPERTY(EditAnywhere)
    float CanvasSize = 1500.f;

    UPROPERTY(VisibleAnywhere)
    UInstancedStaticMeshComponent* InstancedMesh;

private:
    TArray<FCircleData> Circles;
    bool IsOverlapping(const FVector2D& Pos, float Radius) const;
    void TrySpawnNewCircle();

    float TimeAccumulator = 0.f;
	UPROPERTY(EditAnywhere)
	float SimulationStepRate = 0.1f;

    UPROPERTY(EditAnywhere)
    float MinTargetRadius = 1.f;

    UPROPERTY(EditAnywhere)
    float MaxTargetRadius = 300.f;
};
