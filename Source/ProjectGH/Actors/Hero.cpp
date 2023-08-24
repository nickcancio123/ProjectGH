// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/Hero.h"

#include "ProjectGH/Components/GrappleThrustComponent.h"
#include "ProjectGH/Components/GrappleSwingComponent.h"
#include "ProjectGH/Components/CommonGrappleComponent.h"

#include "ObstacleTraversalComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"


#pragma region === Default Actor ===
AHero::AHero()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHero::BeginPlay()
{
	Super::BeginPlay();

	CharacterMovement = GetCharacterMovement();
	MaxRunSpeed = CharacterMovement->MaxWalkSpeed;
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

	// Locomotion
	PlayerInputComponent->BindAxis("MoveForward", this, &AHero::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHero::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AHero::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AHero::LookRight);

	// Sprint
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHero::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHero::StopSprint);

	// Jump
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AHero::TryJump);
	
	// Grappling
	GrappleThrustComp->BindInput(PlayerInputComponent);
	GrappleSwingComp->BindInput(PlayerInputComponent);

	// Obstacle traversal
	ObstacleTraversalComp->BindInput(PlayerInputComponent);
}

void AHero::SetComponentRefs()
{
	CommonGrappleComp = Cast<UCommonGrappleComponent>(GetComponentByClass(UCommonGrappleComponent::StaticClass()));
	GrappleThrustComp = Cast<UGrappleThrustComponent>(GetComponentByClass(UGrappleThrustComponent::StaticClass()));
	GrappleSwingComp = Cast<UGrappleSwingComponent>(GetComponentByClass(UGrappleSwingComponent::StaticClass()));

	ObstacleTraversalComp = Cast<UObstacleTraversalComponent>(GetComponentByClass(UObstacleTraversalComponent::StaticClass()));
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

void AHero::StartSprint()
{
	bSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
}

void AHero::StopSprint()
{
	bSprinting = false;
	CharacterMovement->MaxWalkSpeed = MaxRunSpeed;
}

void AHero::TryJump()
{
	if (GetCharacterMovement()->IsFalling())
		return;

	bJumpTrigger = true;
	
	Jump();
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

void AHero::ResetJumpTrigger()
{
	bJumpTrigger = false;
}

#pragma endregion
