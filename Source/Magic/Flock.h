// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flock.generated.h"

class UFloatingPawnMovement;
class UStaticMeshComponent;
class USphereComponent;

USTRUCT()
struct FAgent {
	GENERATED_BODY()

	FVector position;
	FVector velocity;
	FVector acceleration;
};

UCLASS()
class MAGIC_API AFlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComponent;*/

	FVector Direction;
	FVector StartLocation;

	TArray<TArray<FAgent>> agents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Parameters)
	TSubclassOf<AActor> agentsClass;

	TArray<AActor*> agentObjects;

	int32 currentState;
	int32 previousState;

	FVector FlockPosition;

	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float spawnRadius;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	int cnt;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float maxSpeed;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float minSpeed;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float maxAccel;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float kCoh;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float kSep;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float kAlign;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float kAvoid;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float kFollow;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float rCohesion;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float rSeparation;
	UPROPERTY(EditDefaultsOnly, Category = Parameters)
	float rAlignment;


	void CalcDirection();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
