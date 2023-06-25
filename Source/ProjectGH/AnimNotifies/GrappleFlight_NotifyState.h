// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameFramework/SpringArmComponent.h"
#include "GrappleFlight_NotifyState.generated.h"

class UGrappleComponent;
class AGrapplePoint;

UCLASS()
class PROJECTGH_API UGrappleFlight_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// Time = 0 is start location, Time = 1 is grapple point
	// Value is vertical offset * scale from line between start and grapple point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		FRuntimeFloatCurve PathShapeCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		FRuntimeFloatCurve SpringArmLengthCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		float PathHeightScale = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		float PostGrappleVelocity = 400;
	
	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;

private:
	APawn* Pawn = nullptr;
	UGrappleComponent* GrappleComp = nullptr;
	AGrapplePoint* GrapplePoint = nullptr;
	USpringArmComponent* SpringArm = nullptr;

	float TotalNotifyDuration = 1;
	float RunningTime = 0;

	FVector PathStart, PathEnd, PathDir;
	float PathTotalDist = 0;

	float OriginalSpringArmLength = 400;
};
