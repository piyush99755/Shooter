// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()

		AEnemyController();

private:

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class UBlackboardComponent* BlackboardComponent;

public:

	//its inherit function from AI Controller class 
	virtual void OnPossess(APawn* InPawn) override;

	FORCEINLINE UBehaviorTreeComponent* GetBehaviorTreeComponent() const {return  BehaviorTreeComponent;}
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }



	
};
