// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookupAtRate(float Rate);

	

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	//camera boom positioning camera behind the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess= "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	//variables for turning character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float LookupRate;


	//sound cue variable when firing weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	//particle system variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* FireParticles;

	//spawn particles upon bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		 UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* BeamParticles;

	//animation montage for character while shooting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* HipFireMontage;

	//variables for zoom in functionality
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float CameraZoomedFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float CameraCurrentFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float CameraInterpSpeed; 

	//true if aiming , false if is not aiming 
	bool bAiming; 

	//left mouse button pressed..
	bool bFireButtonPressed;

	//true when we can fire, false when we wait for timer...
	bool bShouldFire;

	//rate of automatic gun fire...
	float AutomaticFireRate;

	FTimerHandle FireTimer;






public:
	FORCEINLINE USpringArmComponent* GetCameraBoom()const { return CameraBoom; }
	

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetAiming() const { return bAiming; }

	//firing weapon functionality
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& WeaponSocketLocation, FHitResult& OutHitResult);

	//handle aiming functionality

	void AimingButtonPressed();

	void AimingButtonReleased();

	void CameraZoomInterp(float DeltaTime);

	//implement automatic fire functionality

	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();


	UFUNCTION()
	void ResetFireTimer();

	

	
};
