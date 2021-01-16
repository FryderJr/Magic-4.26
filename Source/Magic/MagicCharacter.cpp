// Copyright Epic Games, Inc. All Rights Reserved.

#include "MagicCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

//////////////////////////////////////////////////////////////////////////
// AMagicCharacter

AMagicCharacter::AMagicCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create sphere component
	MagicSphere = CreateDefaultSubobject<USphereComponent>(FName("MagicSphere"));
	MagicSphere->SetSphereRadius(2.0f);
	MagicSphere->SetupAttachment(RootComponent);
	MagicSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MagicSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MagicSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	MagicSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Overlap);
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMagicCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMagicCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMagicCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMagicCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMagicCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMagicCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMagicCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMagicCharacter::OnResetVR);
}

void AMagicCharacter::ServerThrowObjects_Implementation()
{
	ThrowObjects();
}

void AMagicCharacter::ThrowObjects()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerThrowObjects();
	}
	TArray<AActor*> OutActors;
	TArray<UPrimitiveComponent*> OutComponents;
	MagicSphere->SetSphereRadius(5000.0);
	MagicSphere->GetOverlappingComponents(OutComponents);
	MagicSphere->GetOverlappingActors(OutActors, APawn::StaticClass());
	if (OutActors.Num() <= 0)
	{
		return;
	}
	FVector Forward = GetActorForwardVector();
	FVector PlayerLocation = GetActorLocation();
	Forward.Normalize();
	AActor* Enemy = nullptr;
	float MinDistance = 5000.0f;
	for (auto& Single : OutActors)
	{
		if (Single == this) {
			continue;
		}
		FVector ToEnemy = Single->GetActorLocation() - GetActorLocation();
		ToEnemy.Normalize();
		float Angle = FMath::Acos(FVector::DotProduct(Forward, ToEnemy)) * 57.2958;
		print(FString::Printf(TEXT("Angle to Enemy %f"), Angle));
		if (Angle < 60.0 && ToEnemy.Size() < MinDistance) {
			Enemy = Single;
			MinDistance = ToEnemy.Size();
		}
	}
	if (Enemy == nullptr)
	{
		return;
	}
	TArray<UPrimitiveComponent*> ThrowObjects;

	for (auto& Single : OutComponents)
	{
		if (Single && Single->IsSimulatingPhysics())
		{
			FVector ToObject = Single->GetComponentLocation() - GetActorLocation();
			ToObject.Normalize();
			float Angle = FMath::Acos(FVector::DotProduct(Forward, ToObject)) * 57.2958;
			if (Angle < 60.0)
			{
				print(FString::Printf(TEXT("Component name is %s"), *Single->GetName()));
				Single->SetEnableGravity(false);
				Single->SetPhysicsLinearVelocity(FVector(0, 0, 160.0));
				Single->SetPhysicsAngularVelocity(FVector(FMath::FRandRange(-100, 100), FMath::FRandRange(-100, 100), FMath::FRandRange(-100, 100)));
				//Single->AddImpulse((Enemy->GetActorLocation() - Single->GetComponentLocation()) * 100);
				ThrowObjects.Emplace(Single);
				Single->SetRenderCustomDepth(true);
				Single->SetCustomDepthStencilValue(2);
			}
		}
	}
	FTimerDelegate CustomDelegate;
	CustomDelegate.BindUFunction(this, FName("AddImpulse"), ThrowObjects, Enemy->GetActorLocation());
	GetWorldTimerManager().SetTimer(FTimeBeforeAttack, CustomDelegate, 0.85, false);
	MagicSphere->SetSphereRadius(2.0);
}

void AMagicCharacter::AddImpulse(TArray<UPrimitiveComponent*> Components, FVector Target)
{
	for (auto& Single : Components)
	{
		Single->AddImpulse((Target - Single->GetComponentLocation()) * 100);
		Single->SetEnableGravity(true);
		Single->SetRenderCustomDepth(false);
	}
}

void AMagicCharacter::ForceWave()
{
	MagicSphere->SetSphereRadius(500.0f);
	TArray<UPrimitiveComponent*> OutComponents;
	MagicSphere->GetOverlappingComponents(OutComponents);
	
	for (auto& Single : OutComponents)
	{
		if (Single)
		{
			AActor* Person = Single->GetOwner();
			print(FString::Printf(TEXT("Component name is %s"), *Single->GetName()));
			if (Person != this && Person != nullptr)
			{
				USkeletalMeshComponent* SMComponent = Cast<USkeletalMeshComponent>(Person->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
				if (SMComponent) {
					/*FTransform DummyTransform;
					DummyTransform.SetLocation(Person->GetActorLocation());
					DummyTransform.SetRotation(Person->GetActorRotation().Quaternion());
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					Person->Destroy();
					AActor* Dummy = GetWorld()->SpawnActor<AActor>(DummyClass, DummyTransform, SpawnParameters);*/
					SMComponent->SetSimulatePhysics(true);
					SMComponent->SetAllBodiesSimulatePhysics(true);
					FVector ToObject = SMComponent->GetComponentLocation() - GetActorLocation();
					SMComponent->AddImpulse(ToObject * 500);
				}
			}
			if (Single->IsSimulatingPhysics())
			{
				FVector ToObject = Single->GetComponentLocation() - GetActorLocation();
				//print(FString::Printf(TEXT("Component name is %s"), *Single->GetName()));
				Single->AddImpulse(ToObject * 500);
			}
		}
	}
	MagicSphere->SetSphereRadius(2.0f);
}

void AMagicCharacter::ThrowFireball()
{
	FTransform FireballTransform;
	FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 55.0;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//SpawnLoc.X += 100.0;
	FireballTransform.SetLocation(SpawnLoc);
	FireballTransform.SetRotation(GetActorRotation().Quaternion());
	AActor *Fireball = GetWorld()->SpawnActor<AActor>(FireballClass, FireballTransform, SpawnParameters);
	print(FString::Printf(TEXT("Fireball name is %s"), *Fireball->GetName()));
	//AActor* Fireballinstance = UGameplayStatics::BeginSpawningActorFromBlueprint(GetWorld(), Fireball, FireballTransform, false);
	//UGameplayStatics::FinishSpawningActor(Fireballinstance, FireballTransform);
}

void AMagicCharacter::CreateShield1()
{
	FTransform ShieldTransform;
	FVector SpawnLoc = GetActorLocation() + GetActorUpVector() * (-15.0);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ShieldTransform.SetLocation(SpawnLoc);
	ShieldTransform.SetRotation(GetActorRotation().Quaternion());
	AActor* Shield = GetWorld()->SpawnActor<AActor>(Shield1Class, ShieldTransform, SpawnParameters);
	//print(FString::Printf(TEXT("Shield name is %s"), *Shield->GetName()));
}

void AMagicCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMagicCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AMagicCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AMagicCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMagicCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMagicCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMagicCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
