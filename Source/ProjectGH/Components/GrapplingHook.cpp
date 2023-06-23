// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrapplingHook.h"

UGrapplingHook::UGrapplingHook()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
}


void UGrapplingHook::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

