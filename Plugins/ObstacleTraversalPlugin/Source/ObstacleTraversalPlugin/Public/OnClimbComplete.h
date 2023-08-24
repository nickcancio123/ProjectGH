// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "OnClimbComplete.generated.h"


class UObstacleTraversalComponent;

UCLASS()
class OBSTACLETRAVERSALPLUGIN_API UOnClimbComplete : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	ACharacter* Character = nullptr;
	UObstacleTraversalComponent* ObstacleTraversalComp = nullptr;
};
