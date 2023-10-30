// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	}

}
