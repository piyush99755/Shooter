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

// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Creates camera boom and declaring its properties 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;//rotate the arm based on controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

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
	
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

		FVector BeamEnd;

		//calling the function where both line trace implemented
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		//if bBeamEnd is true then only spawn impact particles and beam particles
		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				//impact particles at beam end point
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			//even trace hit not hitting any object, still spawn beam particles
			if (BeamParticles)
			{
				//storing spawn emitter value in a particle system component 
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);

				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
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

bool AShooterCharacter::GetBeamEndLocation(const FVector& WeaponSocketLocation, FVector& OutBeamLocation)
{
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
		FHitResult ScreenTraceHit;
		const FVector Start = CrosshairWorldPosition;
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f;

		//set beam end point to line trace end point
		OutBeamLocation = End;

		//single channel line trace, trace outwards from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		//if trace hit blocking  hit, spawn impact particles beam end point which is ScreenTraceHit.location
		if (ScreenTraceHit.bBlockingHit)
		{
			OutBeamLocation = ScreenTraceHit.Location;


		}

		//perform a second trace but this one from gun barrel

		FHitResult WeaponHitResult;
		const FVector WeaponStart = WeaponSocketLocation;
		const FVector WeaponEnd =OutBeamLocation;


		GetWorld()->LineTraceSingleByChannel(WeaponHitResult, WeaponStart, WeaponEnd, ECollisionChannel::ECC_Visibility);
		if (WeaponHitResult.bBlockingHit)
		{
			OutBeamLocation = WeaponHitResult.Location;
		}

		return true;

		
	}

	return false;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

		PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);


}

