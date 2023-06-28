// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GrappleHook_Stick_NotifyState.generated.h"


class AHero;
class UGrappleComponent;
class AGrapplingHook;
class AGrapplePoint;


UCLASS()
class PROJECTGH_API UGrappleHook_Stick_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;


private:
	AHero* Hero = nullptr;
	UGrappleComponent* GrappleComp = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;
	AGrapplePoint* GP = nullptr;
	

	FVector HandPos = FVector::ZeroVector;
	FVector GP_Pos = FVector::ZeroVector;
};
