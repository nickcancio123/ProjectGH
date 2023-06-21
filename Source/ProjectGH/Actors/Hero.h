// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Hero.generated.h"

UCLASS()
class PROJECTGH_API AHero : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
		float CameraTurnSensitivity = 1;


	
	AHero();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	
	UFUNCTION(BlueprintCallable, Category = "Hero")
		FVector GetControlForwardVector();

	UFUNCTION(BlueprintCallable, Category = "Hero")
		FVector GetControlRightVector();

	UFUNCTION(BlueprintCallable, Category = "Hero")
		FVector GetMoveInput();


protected:
	virtual void BeginPlay() override;


private:
	FVector MoveInput = FVector::ZeroVector;

	
	void MoveForward(float InputValue);
	void MoveRight(float InputValue);
	void LookUp(float InputValue);
	void LookRight(float InputValue);
};
