// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplePointContainer.generated.h"

UCLASS()
class PROJECTGH_API AGrapplePointContainer : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrapplePointContainer();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
};
