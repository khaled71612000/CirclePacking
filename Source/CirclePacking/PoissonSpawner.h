#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoissonSpawner.generated.h"

UCLASS()
class CIRCLEPACKING_API APoissonSpawner : public AActor
{
    GENERATED_BODY()

public:
    APoissonSpawner();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere)
    float Radius = 300.f;

    UPROPERTY(EditAnywhere)
    int32 K = 30;

    UPROPERTY(EditAnywhere)
    float CellSize = 0.f;

    UPROPERTY(EditAnywhere)
    float ChunkSize = 3000.f;

    UPROPERTY(EditAnywhere)
    UMaterialInstance* MaterialInstance;

	UPROPERTY()
	TMap<UStaticMesh*, UInstancedStaticMeshComponent*> MeshToInstancer;

    UPROPERTY(EditAnywhere)
    TArray<UStaticMesh*> MeshOptions;

    TArray<FVector2D> Samples;
    TArray<FVector2D> ActiveList;
    TMap<FIntPoint, FVector2D> Grid;

    FVector2D WorldCenter;

    void AddSample(const FVector2D& Point);
    bool IsInNeighborhood(const FVector2D& Point);
    void GenerateNextPoints();

    public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere)
    int32 PointsPerTick = 10;

    float SpawnAccumulator = 0.f;
	UPROPERTY(EditAnywhere)
	float SpawnInterval = 0.1f;
};

