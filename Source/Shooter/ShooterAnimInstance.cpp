// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	//casting character to pawn owner and store its value in ShooterCharacter variable
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	//casting shooter character whenever its nullptr
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		//get character lateral speed from velocity
		FVector Velocity = ShooterCharacter->GetVelocity();
		Velocity.Z = 0; //not using Z direction velocity so set its value 0
		Speed = Velocity.Size();


		//check to see if character is  in air
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();


		//setting bool value of character acclerating or not 
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAcclerating = true;
		}

		else
		{
			bIsAcclerating = false;
		}

		//get base aim rotation of character
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();

		//Using Kismet math library to make rotator from x direction
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());


		//get Movement offset yaw by calculating diffrence between two rotators yaw value 
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, MovementRotation).Yaw;


		//when velocity is zero, it cant calculate movement offset so for job stop blendspace, we need last movementoffsetyaw value
		if (ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		

		
	}

}
