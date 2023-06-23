// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrapplingHook.h"
#include "Components/SphereComponent.h"

UGrapplingHook::UGrapplingHook()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Init detection sphere
	GrapplePointDetector = CreateDefaultSubobject<USphereComponent>(TEXT("Grapple Point Detector"));
	GrapplePointDetector->SetSphereRadius(DetectionRadius);
	GrapplePointDetector->SetRelativeLocation(FVector(DetectionForwardOffset, 0, 0));
	
	GrapplePointDetector->OnComponentBeginOverlap.AddDynamic(this, &UGrapplingHook::OnOverlapStart);
	GrapplePointDetector->OnComponentEndOverlap.AddDynamic(this, &UGrapplingHook::OnOverlapEnd);
}


void UGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
}


void UGrapplingHook::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGrapplingHook::OnRegister()
{
	Super::OnRegister();

	GrapplePointDetector->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}



void UGrapplingHook::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Actor entered"));
	}
}

void UGrapplingHook::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Actor exited"));
	}
}

