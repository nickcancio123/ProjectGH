// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Hero.generated.h"


class UGrapplingComponent;
class UObstacleTraversalComponent;


UCLASS()
class PROJECTGH_API AHero : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero")
		UGrapplingComponent* GrapplingComp = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero")
		UObstacleTraversalComponent* ObstacleTraversalComp = nullptr;
	
 	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero")
		float CameraTurnSensitivity = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero")
		float MaxSprintSpeed = 750;

	UPROPERTY(BlueprintReadOnly, Category = "Hero")
		bool bJumpTrigger = false;

	
	AHero();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UFUNCTION(BlueprintCallable, Category = "Hero")
		void SetComponentRefs();

	UFUNCTION(BlueprintCallable, Category = "Hero")
		void SetupAdvancedMovementComponentSystem();
	
	
	UFUNCTION(BlueprintCallable, Category = "Hero")
		FVector GetControlForwardVector();

	UFUNCTION(BlueprintCallable, Category = "Hero")
		FVector GetControlRightVector();

	UFUNCTION(BlueprintCallable, Category = "Hero")
		FVector GetMoveInput();

	UFUNCTION(BlueprintCallable, Category = "Hero")
		void ResetJumpTrigger();


protected:
	virtual void BeginPlay() override;


private:
	UCharacterMovementComponent* CharacterMovement = nullptr;
	FVector MoveInput = FVector::ZeroVector;
	bool bSprinting = false;
	float MaxRunSpeed = 500;
	
	void MoveForward(float InputValue);
	void MoveRight(float InputValue);
	
	void LookUp(float InputValue);
	void LookRight(float InputValue);

	void StartSprint();
	void StopSprint();

	void TryJump();


	// === Advanced movement component system ===
	// A list of actor components who affect character movement and should be mutually exclusive
	TArray<UActorComponent*> AdvancedMovementComponents; 
	
	UFUNCTION()
	void EnableAllAdvancedMovementComponents();

	void DisableAllAdvancedMovementComponents(UActorComponent* ExceptThisComponent);

	UFUNCTION()
	void OnObstacleTraversalStartEvent();

	UFUNCTION()
	void OnGrapplingStartEvent();
};
