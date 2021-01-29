// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Teams.h"
#include "GameFramework/Character.h"
#include "MagicCharacter.generated.h"

class USphereComponent;

UCLASS(config=Game)
class AMagicCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	FTimerHandle FTimeBeforeAttack;

public:
	AMagicCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(Replicated, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category=Multiplayer)
	TEnumAsByte<ETeams> Team;

	UPROPERTY(VisibleAnywhere, Category=Components)
	USphereComponent *MagicSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Magic)
	TSubclassOf<AActor> FireballClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Magic)
	TSubclassOf<AActor> Shield1Class;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components)
	TSubclassOf<AActor> DummyClass;

	UFUNCTION(Server, Reliable)
	void ServerThrowObjects();

	/* Throws all surrounding objects to the enemy in front of player */
	UFUNCTION(BlueprintCallable, Category=Magic)
	void ThrowObjects();

	UFUNCTION()
	void AddImpulse(TArray<UPrimitiveComponent*> Components, FVector Target);

	UFUNCTION(BlueprintCallable, Category=Magic)
	void ForceWave();

	UFUNCTION(BlueprintCallable, Category=Magic)
	void ThrowFireball();

	UFUNCTION(BlueprintCallable, Category = Magic)
	void CreateShield1();

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

