// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrappleSwingComponent.h"

UGrappleSwingComponent::UGrappleSwingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UGrappleSwingComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGrappleSwingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

