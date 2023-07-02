// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleSwingComponent.generated.h"



UENUM(BlueprintType)
enum EGrappleSwingState
{
	GSS_Idle = 0,
	GSS_Throw = 1,
	GSS_Swing = 3
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTGH_API UGrappleSwingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGrappleSwingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;		
};
