// Fill out your copyright notice in the Description page of Project Settings.


#include "Flock.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/RotationMatrix.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

// Sets default values
AFlock::AFlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	FlockPosition = FVector::ZeroVector;
	/*SphereComponent = CreateDefaultSubobject<USphereComponent>(FName("SphereComp"));
	SphereComponent->SetSphereRadius(200);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	SphereComponent->SetupAttachment(RootComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("MeshComp"));
	MeshComponent->SetCanEverAffectNavigation(false);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetupAttachment(RootComponent);

	MaxSpeed = 24.5f;*/

}

// Called when the game starts or when spawned
void AFlock::BeginPlay()
{
	Super::BeginPlay();
	
	
	for (int i = 0; i < cnt; i++)
	{
		FTransform agentTransform;
		FActorSpawnParameters agentSpawnParams;
		agentTransform.SetLocation(GetActorLocation() +
			FVector(
				FMath::RandRange(-spawnRadius, spawnRadius),
				FMath::RandRange(-spawnRadius, spawnRadius),
				FMath::RandRange(-spawnRadius, spawnRadius)
			)
		);
		agentTransform.SetRotation(GetActorRotation().Quaternion());
		agentSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
		AActor* Agent = GetWorld()->SpawnActor<AActor>(agentsClass, agentTransform, agentSpawnParams);
		if (Agent != nullptr)
		{
			TArray<FAgent> arr;
			FAgent one;
			agentObjects.AddUnique(Agent);
			one.position = agentObjects.Last()->GetActorLocation();
			FlockPosition += one.position;
			//one.velocity = agentObjects.Last()->GetActorForwardVector();
			print(FString::Printf(TEXT("agent`s %i position is %f %f %f"),
				i,
				one.position.X,
				one.position.Y,
				one.position.Z));
			arr.Add(one);
			arr.Add(one);
			agents.Add(arr);
		}
	}
	cnt = agents.Num();
	FlockPosition /= cnt;
	print(FString::Printf(TEXT("agents count is %i"), agents.Num()));
	currentState = 1;
	previousState = 0;
}

void AFlock::CalcDirection()
{
	
}

// Called every frame
void AFlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	int32 currentStateIndex = currentState;
	int32 previousStateIndex = previousState;

	auto tempAgents = agents;

	AActor* BestTargetToAttack = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator iterator = GetWorld()->GetPawnIterator(); iterator; iterator++)
	{
		APawn* TestPawn = iterator->Get();
		if (TestPawn == nullptr)
		{
			continue;
		}
		float Distance = (FlockPosition - TestPawn->GetActorLocation()).Size();
		if (NearestTargetDistance > Distance)
		{
			BestTargetToAttack = TestPawn;
			NearestTargetDistance = Distance;
		}
	}

	ParallelFor(cnt, [&tempAgents, &BestTargetToAttack, currentStateIndex, previousStateIndex, DeltaTime, this](int32 agentNum)
		{
			FVector 
				cohesion(FVector::ZeroVector),
				separation(FVector::ZeroVector),
				alignment(FVector::ZeroVector),
				follow(FVector::ZeroVector),
				flockPosition(FVector::ZeroVector);
			int32 cohesionCnt = 0, separationCnt = 0, alignmentCnt = 0;
			for (int i = 0; i < cnt; i++)
			{
				flockPosition += tempAgents[i][currentStateIndex].position;
				if (i != agentNum)
				{
					float distance = FVector::Distance(tempAgents[i][currentStateIndex].position,
						tempAgents[agentNum][currentStateIndex].position);
					follow = BestTargetToAttack->GetActorLocation() - tempAgents[agentNum][currentStateIndex].position;
					follow.Normalize();
					if (distance < rCohesion)
					{
						cohesion += tempAgents[i][currentStateIndex].position;
						cohesionCnt++;
					}
					if (distance < rSeparation)
					{
						separation += tempAgents[i][currentStateIndex].position - tempAgents[agentNum][currentStateIndex].position;
						separationCnt++;
					}
					if (distance < rAlignment)
					{
						alignment += tempAgents[i][currentStateIndex].velocity;
						alignmentCnt++;
					}
					if (cohesionCnt != 0)
					{
						cohesion /= cohesionCnt;
						cohesion -= tempAgents[agentNum][currentStateIndex].position;
						cohesion.Normalize();
					}
					if (separationCnt != 0)
					{
						separation /= separationCnt;
						separation *= -1.f;
						separation.Normalize();
					}
					if (alignmentCnt != 0)
					{
						alignment /= alignmentCnt;
						alignment.Normalize();
					}
					tempAgents[agentNum][currentStateIndex].acceleration = (cohesion * kCoh + 
						separation * kSep + alignment * kAlign + follow * kFollow);
					/*print(FString::Printf(TEXT("agent`s %i position is %f %f %f"),
						agentNum,
						tempAgents[agentNum][currentStateIndex].acceleration.X,
						tempAgents[agentNum][currentStateIndex].acceleration.Y,
						tempAgents[agentNum][currentStateIndex].acceleration.Z));*/
					tempAgents[agentNum][currentStateIndex].velocity += tempAgents[agentNum][currentStateIndex].acceleration.GetClampedToMaxSize(maxAccel) * DeltaTime;
					tempAgents[agentNum][currentStateIndex].velocity =
						tempAgents[agentNum][currentStateIndex].velocity.GetClampedToMaxSize(maxSpeed);
					tempAgents[agentNum][currentStateIndex].position += tempAgents[agentNum][currentStateIndex].velocity * DeltaTime;
				}
			}
			FlockPosition = flockPosition / cnt;
			/*FHitResult hit(ForceInit);
			FCollisionShape sphereShape;
			FCollisionQueryParams queryParams;
			queryParams.AddIgnoredActors(agentObjects);
			//sphereShape.MakeSphere(100.0f);
			sphereShape.SetSphere(20.0f);
			if (GetWorld()->SweepSingleByChannel(hit,
				tempAgents[agentNum][currentStateIndex].position,
				tempAgents[agentNum][currentStateIndex].position +
				tempAgents[agentNum][currentStateIndex].velocity / DeltaTime * 0.5,
				FQuat(0, 0, 0, 0),
				ECollisionChannel::ECC_Visibility, sphereShape, queryParams))
			{
				tempAgents[agentNum][currentStateIndex].position -= tempAgents[agentNum][currentStateIndex].velocity * DeltaTime;
				//tempAgents[j][currentStateIndex].velocity *= -1;
				tempAgents[agentNum][currentStateIndex].velocity += hit.ImpactNormal * kAvoid;
				tempAgents[agentNum][currentStateIndex].velocity.GetClampedToMaxSize(minSpeed);
				tempAgents[agentNum][currentStateIndex].position += tempAgents[agentNum][currentStateIndex].velocity * DeltaTime;
			}*/
		}, false);
	agents = tempAgents;
	for (int j = 0; j < cnt; j++)
	{
		FHitResult hit(ForceInit);
		FVector direction = tempAgents[j][currentStateIndex].velocity;
		direction.Normalize();
		FCollisionShape sphereShape;
		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActors(agentObjects);
		sphereShape.SetSphere(20.0f);
		/*DrawDebugLine(GetWorld(), tempAgents[j][currentStateIndex].position,
			tempAgents[j][currentStateIndex].position +
			direction * 150.0f, FColor::Red);*/
		if (GetWorld()->SweepSingleByChannel(hit,
			tempAgents[j][currentStateIndex].position,
			tempAgents[j][currentStateIndex].position + 
			direction * 150.0f,
			FQuat(0, 0, 0, 0),
			ECollisionChannel::ECC_Visibility, sphereShape, queryParams))
		{
			tempAgents[j][currentStateIndex].position -= tempAgents[j][currentStateIndex].velocity * DeltaTime;
			//tempAgents[j][currentStateIndex].velocity *= -1;
			//tempAgents[j][currentStateIndex].velocity = tempAgents[j][currentStateIndex].velocity.MirrorByVector(hit.ImpactNormal);
			tempAgents[j][currentStateIndex].velocity += hit.ImpactNormal * kAvoid;
			tempAgents[j][currentStateIndex].velocity.GetClampedToMaxSize(maxSpeed);
			tempAgents[j][currentStateIndex].position += tempAgents[j][currentStateIndex].velocity * DeltaTime;
		}
		FTransform agentTransform;
		direction = tempAgents[j][currentStateIndex].velocity;
		direction.Normalize();
		agentTransform.SetLocation(tempAgents[j][currentStateIndex].position);
		agentTransform.SetRotation(FRotationMatrix::MakeFromX(direction).Rotator().Quaternion());
		agentObjects[j]->SetActorTransform(agentTransform);
		agents[j][previousState] = tempAgents[j][currentState];
		agents[j][currentState] = tempAgents[j][currentState];
	}
	/*for (int j = 0; j < cnt; j++)
	{
		FTransform agentTransform;
		FVector direction = tempAgents[j][currentStateIndex].velocity;
		direction.Normalize();
		agentTransform.SetLocation(tempAgents[j][currentStateIndex].position);
		agentTransform.SetRotation(FRotationMatrix::MakeFromX(direction).Rotator().Quaternion());
		agentObjects[j]->SetActorTransform(agentTransform);
		agents[j][previousState] = tempAgents[j][currentState];
		agents[j][currentState] = tempAgents[j][currentState];
	}*/
	/*int32 tempState = currentState;
	currentState = previousState;
	previousState = tempState;*/
}