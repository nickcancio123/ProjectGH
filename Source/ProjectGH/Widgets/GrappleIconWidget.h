// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GrappleIconWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTGH_API UGrappleIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FVector2D IconPositionOffset = FVector2D(-10, -10);	
};
