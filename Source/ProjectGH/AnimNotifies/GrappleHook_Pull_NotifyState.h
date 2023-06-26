// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GrappleHook_Pull_NotifyState.generated.h"



class AHero;
class UGrappleComponent;
class AGrapplingHook;


UCLASS()
class PROJECTGH_API UGrappleHook_Pull_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// The percent of the way back the hook is pulled toward hand
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grappling")
		float PullBackPercent = 0.7f;
	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;


private:
	AHero* Hero = nullptr;
	UGrappleComponent* GrappleComp = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;
	
	float NotifyTotalDuration = 1;
	float RunningTime = 0;

	FVector HandPos = FVector::ZeroVector;
	FVector GP_Pos = FVector::ZeroVector;
	FVector PathDir = FVector::ZeroVector;
	float TotalPathDist = 0;
};
