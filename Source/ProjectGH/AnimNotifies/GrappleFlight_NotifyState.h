// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameFramework/SpringArmComponent.h"
#include "GrappleFlight_NotifyState.generated.h"

class UCommonGrappleComponent;
class UGrappleThrustComponent;
class AGrapplePoint;
class AHero;

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

	// Percent of grapple speed that is retained after grapple animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		float PercentSpeedRetainedPostGrapple = 0.5;

	// Percent of grapple speed that can be applied through input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		float PercentSpeedInputSpeed = 0.4;
	
	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;

private:
	AHero* Hero = nullptr;
	UCommonGrappleComponent* CommonGrappleComp = nullptr;
	UGrappleThrustComponent* GrappleThrustComp = nullptr;
	AGrapplePoint* GrapplePoint = nullptr;
	USpringArmComponent* SpringArm = nullptr;

	float TotalNotifyDuration = 1;
	float RunningTime = 0;

	FVector PathStart, PathEnd;

	float OriginalSpringArmLength = 400;
};
