// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbStateMovement.h"
#include "ObstacleTraversalComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"



void UClimbStateMovement::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	TotalNotifyDuration = TotalDuration;
	RunningTime = 0;
	
	Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
		return;
	
	ObstacleTraversalComp = Cast<UObstacleTraversalComponent>(Character->GetComponentByClass(UObstacleTraversalComponent::StaticClass()));
	if (!ObstacleTraversalComp)
		return;

	// Compute start and target positions and rotations
	StartPos = Character->GetActorLocation();
	TargetPos = ObstacleTraversalComp->GetTargetClimbMountPosition();
	
	StartRot = Character->GetActorRotation();
	TargetRot = ObstacleTraversalComp->GetTargetClimbRotation();
	
	
	// Set movement mode
	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	

	// Disable collision
	CapsuleComp = Cast<UCapsuleComponent>(Character->GetComponentByClass(UCapsuleComponent::StaticClass()));
	if (CapsuleComp)
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (!MeshComp)
		return;

	// Disable root motion
	AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return;

	InitialRootMotionMode = AnimInstance->RootMotionMode;
	AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
}

void UClimbStateMovement::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);
	
	if (!Character)
		return;
	
	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / TotalNotifyDuration;
	
	// Lerp position
	FVector NewPos = FMath::Lerp(StartPos, TargetPos, Alpha);
	Character->SetActorLocation(NewPos);
	
	// Lerp rotation
	FRotator NewRot = FMath::Lerp(StartRot, TargetRot, Alpha);
	Character->SetActorRotation(NewRot);
}

void UClimbStateMovement::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	
	if (!Character)
		return;

	// Reset root motion settings
	if (AnimInstance)
		AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
}