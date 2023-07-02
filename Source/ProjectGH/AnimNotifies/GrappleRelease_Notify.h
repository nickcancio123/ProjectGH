// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GrappleRelease_Notify.generated.h"


class AHero;
class UGrappleThrustComponent;

UCLASS()
class PROJECTGH_API UGrappleRelease_Notify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;;

private:
	AHero* Hero = nullptr;
	UGrappleThrustComponent* GrappleThrustComp = nullptr;
};
