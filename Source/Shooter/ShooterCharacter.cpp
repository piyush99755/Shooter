// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "BulletHitInterface.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Creates camera boom and declaring its properties 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->bUsePawnControlRotation = true;//rotate the arm based on controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//camera that follows character
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//stop character from rotating along with controller rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//configure character movement
	//character moves in the direction of input
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//variable for turn and lookup
	TurnRate = 45.f;
	LookupRate = 45.f;

	CameraDefaultFOV = 0.f;
	CameraZoomedFOV = 60.f;
	
	
	bAiming = false;
	CameraInterpSpeed = 20.f;

	AutomaticFireRate = 0.1f;
	bShouldFire = true;
	bFireButtonPressed = false;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	//getting default value of Camera FOV

	CameraDefaultFOV = GetFollowCamera()->FieldOfView;

	//set camera current FOV to Camera Default FOV..
	CameraCurrentFOV = CameraDefaultFOV;

	
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.f))
	{
		//find out which direction is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction * Value);
	}
	
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.f))
	{
		//find out which direction is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction * Value);
	}
	

}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookupAtRate(float Rate)
{
	AddControllerPitchInput(Rate * LookupRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::FireWeapon()
{
	//play sound when firing weapon
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), FireSound);
	}

	//get socket transform for spawn emitter at location function 
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (FireParticles)
		{
			//spawning particles while firing weapon
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireParticles, SocketTransform);
		}

		FHitResult BeamHitResult;

		//calling the function where both line trace implemented
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);

		//if bBeamEnd is true then only spawn impact particles and beam particles
		if (bBeamEnd)
		{
			//does actor hit bullet hit interface?
			if (BeamHitResult.Actor.IsValid())
			{
				
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());

				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult);
				}
			}

			else
			{
				if (ImpactParticles)
				{
					//impact particles at beam end point
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}
			}

			

			//even trace hit not hitting any object, still spawn beam particles
			if (BeamParticles)
			{
				//storing spawn emitter value in a particle system component 
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);

				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
				}
			}
		}

	}

	//getting reference of anim instance which has function to play animation montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("WeaponFire"));
	}
	
	
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& WeaponSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;
	FVector2D ViewportSize;

	//getting game view port size
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//getting crosshair location by dividing x and y axis of viewport by 2.f
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f; // raising crosshair from center by 50.f

	//getting crosshair world position and direction
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bWorldToScreen = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bWorldToScreen)
	{
		
		const FVector Start = CrosshairWorldPosition;
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f;

		//set beam end point to line trace end point
		OutBeamLocation = End;

		//single channel line trace, trace outwards from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		//if trace hit blocking  hit, spawn impact particles beam end point which is ScreenTraceHit.location
		if (!OutHitResult.bBlockingHit)
		{
			OutHitResult.Location = OutBeamLocation;


		}

		//perform a second trace but this one from gun barrel

		
		const FVector WeaponStart = WeaponSocketLocation;
		const FVector WeaponEnd =OutBeamLocation;


		GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponStart, WeaponEnd, ECollisionChannel::ECC_Visibility);
		if (!OutHitResult.bBlockingHit)
		{
			OutHitResult.Location = OutBeamLocation;
			return false;
		}

		return true;

		
	}

	return false;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//handle camera interpolation while aiming
	CameraZoomInterp(DeltaTime);
	
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent)

		PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	    PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
		PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::TurnAtRate);
		PlayerInputComponent->BindAxis("Lookup", this, &AShooterCharacter::LookupAtRate);

		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
		PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

		PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
		PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

		PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
		PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShooterCharacter::AimingButtonReleased);


}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;

	//setting up field of view
	GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
}

void AShooterCharacter::AimingButtonReleased()
{ 
	bAiming = false;

	//setting up field of view
	GetFollowCamera()->SetFieldOfView(CameraDefaultFOV);
}
void AShooterCharacter::CameraZoomInterp(float DeltaTime)
{
	// is aiming?
	if (bAiming)
	{
		//interpolate to camera zoom FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, CameraInterpSpeed);

	}
	else
	{
		//interpolate to Camera Default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, CameraInterpSpeed);

	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;

	//this function handles fire functionality along with setting and resetting timer...
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		//if should fire is true fire weapon and set bool to false to prevent from firing..
		FireWeapon();
		bShouldFire = false;

		//start fire timer.
		GetWorldTimerManager().SetTimer(FireTimer, this, &AShooterCharacter::ResetFireTimer, AutomaticFireRate);

	}
}

void AShooterCharacter::ResetFireTimer()
{
	//set boolto true 
	bShouldFire = true;

	//if fire button is pressed after firing once, call start fire timers to start firing weapon again 
	if (bFireButtonPressed)
	{
		
		StartFireTimer();
	}
}


