// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrapplingComponent.h"

UGrapplingComponent::UGrapplingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UGrapplingComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGrapplingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

