// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "ObstacleTraversalComponent.h"

#include "ClimbStateMovement.generated.h"

/**
 * 
 */
UCLASS()
class OBSTACLETRAVERSALPLUGIN_API UClimbStateMovement : public UAnimNotifyState
{
	GENERATED_BODY()


protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	ACharacter* Character = nullptr;
	UCapsuleComponent* CapsuleComp = nullptr;
	UObstacleTraversalComponent* ObstacleTraversalComp = nullptr;

	TEnumAsByte<ERootMotionMode::Type> InitialRootMotionMode = ERootMotionMode::RootMotionFromEverything;

	UAnimInstance* AnimInstance = nullptr;
	
	FVector StartPos = FVector::ZeroVector;	
	FVector TargetPos = FVector::ZeroVector;
	
	FRotator StartRot = FRotator::ZeroRotator;
	FRotator TargetRot = FRotator::ZeroRotator;
	
	float TotalNotifyDuration = 0;
	float RunningTime = 0;
};
