// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/Hero.h"
#include "DrawDebugHelpers.h"


#pragma region === Default Actor ===
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

	MoveInput.Normalize();
	FVector WorldMoveInput = (MoveInput.X * GetControlForwardVector()) + (MoveInput.Y * GetControlRightVector());
	AddMovementInput(WorldMoveInput);
}

void AHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHero::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHero::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AHero::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AHero::LookRight);
}

#pragma endregion



#pragma region === Locomotion ===
void AHero::MoveForward(float InputValue)
{
	MoveInput.X = InputValue;
}

void AHero::MoveRight(float InputValue)
{
	MoveInput.Y = InputValue;
}

void AHero::LookUp(float InputValue)
{
	AddControllerPitchInput(InputValue * CameraTurnSensitivity);
}

void AHero::LookRight(float InputValue)
{
	AddControllerYawInput(InputValue * CameraTurnSensitivity);
}
#pragma endregion



#pragma region === Helper ===
// Returns the horizontal/flat/XY forward vector of the control rotation
FVector AHero::GetControlForwardVector()
{
	// Compute forward vector of control rotation by projecting control rotation vector onto horizontal plane
	FVector Forward = FVector::VectorPlaneProject(GetControlRotation().Vector(), FVector::UpVector);
	return Forward.GetSafeNormal();
}

// Returns the horizontal/flat/XY right vector of the control rotation
FVector AHero::GetControlRightVector()
{
	// Compute right vector of control rotation by taking cross product of control rotation vector and world up vector
	FVector Right = FVector::CrossProduct(FVector::UpVector, GetControlRotation().Vector());
	return Right.GetSafeNormal();
}

// Returns current frame's XY movement input
FVector AHero::GetMoveInput()
{
	return MoveInput.GetSafeNormal();
}

#pragma endregion