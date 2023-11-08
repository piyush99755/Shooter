// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Actor.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//constructing agro sphere and setting up its radius 
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->SetSphereRadius(600.f);

	//constructing enemy's attack sphere 
	AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
	AttackSphere->SetupAttachment(GetRootComponent());
	AttackSphere->SetSphereRadius(300.f);


	//in beginning of game value is set to true
	bCanHitReact = true; 

	MinHitReactTime = 0.5f;
	MaxHitReactTime = 0.75f;

	bStunned = false;
	StunChance = 0.5f;
	

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	//binding callback function to OnComponentBeginOverlap event
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	AttackSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AttackSphereOverlap);

	//set collsion reponse to block, so enemy block bullets from shooter
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	//transform patrol point from local to world
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);

	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Yellow, true);

	DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Yellow, true);

	//casting enemy controller
	 EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyController)
	{
		//setting black board key value
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);

		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);

		//it will excute nodes in behavior tree 
		EnemyController->RunBehaviorTree(BehaviorTree);
	}
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{    

	//spawn particles when bullet hit
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, HitResult.Location, FRotator(0.f), true);
	}


	//play sound when bullet hit
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	
    const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
			//play hit montage when bullet is hitting enemy 
		PlayHitMontage(FName("HitReactFront"));
		SetStunned(true);
	}
		

	
	
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		//casting to shooter character and set value as object in blackboaard
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

		if (ShooterCharacter)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), ShooterCharacter);
		}
	}
}

void AEnemy::AttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EnemyHitMontage)
		{
			AnimInstance->Montage_Play(EnemyHitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, EnemyHitMontage);
		}
    }

	bCanHitReact = false;

	const float HitReactTime = FMath::FRandRange(MinHitReactTime, MaxHitReactTime);
	 //set timer
	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer,HitReactTime);
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true; 
}

void AEnemy::SetStunned(bool Stunned)
{
	
	bStunned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}
	

	
	
	
	
}

