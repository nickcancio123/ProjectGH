// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/GrapplePointContainer.h"

AGrapplePointContainer::AGrapplePointContainer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGrapplePointContainer::BeginPlay()
{
	Super::BeginPlay();
}

void AGrapplePointContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

