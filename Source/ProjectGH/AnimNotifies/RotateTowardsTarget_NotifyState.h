// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RotateTowardsTarget_NotifyState.generated.h"


UENUM()
enum ETargetType
{
	ViewDirection     UMETA(DisplayName = "View Direction"),
	GrapplePoint     UMETA(DisplayName = "Grapple Point")
};

UCLASS()
class PROJECTGH_API URotateTowardsTarget_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notify")
		TEnumAsByte<ETargetType> TargetType = ETargetType::ViewDirection;	
	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;


private:
	APawn* Pawn = nullptr;
	
	FRotator StartRot;
	FRotator TargetRot;
	
	float NotifyTotalDuration = 1;
	float RunningTime = 0;
};
