// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "ObstacleTraversalComponent.h"

#include "VaultStateMovement.generated.h"


class UCapsuleComponent;

UCLASS()
class OBSTACLETRAVERSALPLUGIN_API UVaultStateMovement : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting")
	TEnumAsByte<EVaultState> CurrentVaultState = EVaultState::VS_MOUNT;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting")
	bool bLastStateOfVault = false;
	
protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	ACharacter* Character = nullptr;
	UCharacterMovementComponent* CharacterMovementComp = nullptr;
	UCapsuleComponent* CapsuleComp = nullptr;
	UObstacleTraversalComponent* ObstacleTraversalComp = nullptr;

	TEnumAsByte<ERootMotionMode::Type> InitialRootMotionMode = ERootMotionMode::RootMotionFromEverything;

	FVector StartPos = FVector::ZeroVector;	
	FVector TargetPos = FVector::ZeroVector;

	FRotator StartRot = FRotator::ZeroRotator;
	FRotator TargetRot = FRotator::ZeroRotator;
	
	float TotalNotifyDuration = 0;
	float RunningTime = 0;
};
