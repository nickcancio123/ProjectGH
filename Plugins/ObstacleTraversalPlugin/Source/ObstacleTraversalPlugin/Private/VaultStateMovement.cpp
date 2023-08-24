// Fill out your copyright notice in the Description page of Project Settings.


#include "VaultStateMovement.h"
#include "ObstacleTraversalComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"


void UVaultStateMovement::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
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
	
	StartPos = Character->GetActorLocation();
	TargetPos = ObstacleTraversalComp->GetTargetVaultPosition(CurrentVaultState);

	StartRot = Character->GetActorRotation();
	TargetRot = ObstacleTraversalComp->GetTargetVaultRotation();


	// Set movement mode
	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	// Disable collision
	CapsuleComp = Cast<UCapsuleComponent>(Character->GetComponentByClass(UCapsuleComponent::StaticClass()));
	if (CapsuleComp)
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UVaultStateMovement::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
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

void UVaultStateMovement::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!Character)
		return;
	
	if (bLastStateOfVault && ObstacleTraversalComp)
		ObstacleTraversalComp->OnVaultComplete();
}
