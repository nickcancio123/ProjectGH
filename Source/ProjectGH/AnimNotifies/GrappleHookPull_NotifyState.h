// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GrappleHookPull_NotifyState.generated.h"


class AHero;

class UCommonGrappleComponent;
class UGrappleThrustComponent;
class UGrappleSwingComponent;

class AGrapplingHook;
class AGrapplePoint;

UCLASS()
class PROJECTGH_API UGrappleHookPull_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grapple Pull")
		float MaxLerpAlpha = 0.75;
	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;


private:
	AHero* Hero = nullptr;
	
	UCommonGrappleComponent* CommonGrappleComp = nullptr;
	UGrappleThrustComponent* GrappleThrustComp = nullptr;
	UGrappleSwingComponent* GrappleSwingComp = nullptr;
	
	AGrapplingHook* GrapplingHook = nullptr;
	AGrapplePoint* GrapplePoint = nullptr;
	
	float NotifyTotalDuration = 1;
	float RunningTime = 0;

	FVector HandPos = FVector::ZeroVector;
	FVector GP_Pos = FVector::ZeroVector;
};
