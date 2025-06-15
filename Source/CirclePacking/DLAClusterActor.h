#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DLAClusterActor.generated.h"

USTRUCT()
struct FWalker
{
    GENERATED_BODY()

    FIntVector Position;

    FWalker() : Position(FIntVector::ZeroValue) {}
    FWalker(FIntVector InPosition) : Position(InPosition) {}
};

//Spawns a bunch of invisible “walkers”(agents).
//These walkers randomly move around in 3D.
//If a walker touches the crystal, it becomes part of it(and spawns a new walker on the edge).
//The crystal keeps growing, like mold or snowflakes

UCLASS()
class CIRCLEPACKING_API ADLAClusterActor : public AActor
{
 GENERATED_BODY()

public:
    ADLAClusterActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void SimulateStep();
    bool IsAdjacentToAggregate(const FIntVector& Pos) const;
    FIntVector GetRandomEdgePosition() const;
    void AddInstanceToMesh(const FIntVector& Pos);

    UPROPERTY(EditAnywhere)
    int32 MaxWalkers = 200;

    UPROPERTY(EditAnywhere)
    int32 Bounds = 50;


    //Converts grid units to world units.
    //E.g., cube at(2, 1, 0) → world pos = (200, 100, 0)
    UPROPERTY(EditAnywhere)
    float GridSpacing = 100.0f;
    //List of agents that move randomly
    TArray<FWalker> Walkers;
    //Stores all cubes that are part of the growing crystal
    TSet<FIntVector> AggregatePoints;

    UPROPERTY(VisibleAnywhere)
    UInstancedStaticMeshComponent* MeshComponent;

    
    float TimeAccumulator = 0.f;
	UPROPERTY(EditAnywhere)
	float SimulationStepRate = 0.1f;
};
