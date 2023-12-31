// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	
private:
	//variable for spawn particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* HitParticles;

	//variable to play sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USoundCue* HitSound;

	//behavior tree for AI character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	//variable for patrol point, which is local to enemy. MakeEditWidget helps for transformation such as translation and rotation in the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta= (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	//patrol point 2 for enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
		FVector PatrolPoint2;

	
	//enemy controller
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTree", meta = (AllowPrivateAccess = "true"))
	class AEnemyController* EnemyController;

	//Sphere component for enemy as collision 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* AgroSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USphereComponent* AttackSphere;

	//montage containing hit and dealth animations 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* EnemyHitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* EnemyAttackMontage;
	//variables to delay hit react animation..

	FTimerHandle HitReactTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bCanHitReact; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float MinHitReactTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float MaxHitReactTime;

	//variables for stun behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bStunned; 

	//stun chance of enemy on bullet hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float StunChance; 

	//bool to check if player is in attack range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange; 

	//attack section names
	FName AttackLAFast;
	FName AttackRAFast;
	FName AttackLA;
	FName AttackRA;
	



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//function inherit from BulletHit Interface
	virtual void BulletHit_Implementation(FHitResult HitResult)override;

	//getter for behavior tree
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	UFUNCTION()
		void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void AttackSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void AttackSphereOverlapEnd( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	 
	//function to play enemy hit montage
	void PlayHitMontage(FName Section, float PlayRate = 1.0f);


	//function to play attack anim montage
	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate = 1.f);

	//function to get attack section name from enemy attack montage
	UFUNCTION(BlueprintPure)
		FName GetAttackSectionName();

	void ResetHitReactTimer();
	
	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

};
