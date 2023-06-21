// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/Hero.h"

// Sets default values
AHero::AHero()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHero::BeginPlay()
{
	Super::BeginPlay();
}

void AHero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHero::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHero::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AHero::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AHero::LookRight);
}



void AHero::MoveForward(float InputValue)
{
	// Compute forward vector of control rotation by projecting control rotation vector onto horizontal plane
	FVector ControlForward =
		FVector::VectorPlaneProject(GetControlRotation().Vector(), FVector::UpVector);

	AddMovementInput(ControlForward * InputValue);
}

void AHero::MoveRight(float InputValue)
{
	// Compute right vector of control rotation by taking cross product of control rotation vector and world up vector
	FVector ControlRight =
		FVector::CrossProduct(FVector::UpVector, GetControlRotation().Vector());

	AddMovementInput(ControlRight * InputValue);
}

void AHero::LookUp(float InputValue)
{
	AddControllerPitchInput(InputValue * CameraTurnSensitivity);
}

void AHero::LookRight(float InputValue)
{
	AddControllerYawInput(InputValue * CameraTurnSensitivity);
}