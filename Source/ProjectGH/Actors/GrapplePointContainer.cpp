// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/GrapplePointContainer.h"
#include "Components/BoxComponent.h"


AGrapplePointContainer::AGrapplePointContainer()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AGrapplePointContainer::OnOverlapStart);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AGrapplePointContainer::OnOverlapEnd);
}

void AGrapplePointContainer::BeginPlay()
{
	Super::BeginPlay();
}

void AGrapplePointContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGrapplePointContainer::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor entered volume!"));
	}
}

void AGrapplePointContainer::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor exited volume!"));
	}
}


