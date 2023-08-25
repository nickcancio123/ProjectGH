// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameFramework/SpringArmComponent.h"
#include "GrappleThrust_NotifyState.generated.h"


//class UCommonGrappleComponent;
//class UGrappleThrustComponent;
class UGrapplingComponent;

class AGrapplePoint;
class AGrapplingHook;


UCLASS()
class PROJECTGH_API UGrappleThrust_NotifyState : public UAnimNotifyState
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

	// Percent of grapple thrust speed that is retained after grapple animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		float PostThrustSpeedRetained = 0.2;

	// Percent of grapple speed that can be applied through input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
		float PostThrustInputImpulseSpeed = 0.15;
	
	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;

private:
	ACharacter* Character = nullptr;
	
	//UCommonGrappleComponent* CommonGrappleComp = nullptr;
	//UGrappleThrustComponent* GrappleThrustComp = nullptr;
	UGrapplingComponent* GrapplingComp = nullptr;
	
	AGrapplingHook* GrapplingHook = nullptr;
	AGrapplePoint* GrapplePoint = nullptr;
	USpringArmComponent* SpringArm = nullptr;

	float TotalNotifyDuration = 1;
	float RunningTime = 0;

	FVector PathStart, PathEnd;

	float OriginalSpringArmLength = 400;
};
