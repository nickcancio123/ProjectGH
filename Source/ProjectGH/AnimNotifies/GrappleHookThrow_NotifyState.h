// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ProjectGH/Components/CommonGrappleComponent.h"
#include "GrappleHookThrow_NotifyState.generated.h"



//class UCommonGrappleComponent;
//class UGrappleThrustComponent;
//class UGrappleSwingComponent;

class UGrapplingComponent;

class AGrapplingHook;
class AGrapplePoint;


UCLASS()
class PROJECTGH_API UGrappleHookThrow_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;


private:
	ACharacter* Character = nullptr;
	
	//UCommonGrappleComponent* CommonGrappleComp = nullptr;
	//UGrappleThrustComponent* GrappleThrustComp = nullptr;
	//UGrappleSwingComponent* GrappleSwingComp = nullptr;

	UGrapplingComponent* GrapplingComp = nullptr;
	
	AGrapplingHook* GrapplingHook = nullptr;
	AGrapplePoint* GrapplePoint = nullptr;
	
	float NotifyTotalDuration = 1;
	float RunningTime = 0;

	FVector HandPos = FVector::ZeroVector;
	FVector GP_Pos = FVector::ZeroVector;
};
