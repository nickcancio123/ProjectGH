// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrapplePointDetectorComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "Components/SphereComponent.h"



#pragma region Default Actor Component Methods
UGrapplePointDetectorComponent::UGrapplePointDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	DetectionVolume = CreateDefaultSubobject<USphereComponent>("Detection Volume");
}

void UGrapplePointDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOverlappedGrapplePoints();
}

void UGrapplePointDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGrapplePointDetectorComponent::OnRegister()
{
	Super::OnRegister();
	InitDetectionVolume();
}
#pragma endregion


void UGrapplePointDetectorComponent::InitDetectionVolume()
{
	DetectionVolume->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	DetectionVolume->SetRelativeLocation(FVector::ZeroVector);
	
	DetectionVolume->SetSphereRadius(DetectionRadius);
	DetectionVolume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	DetectionVolume->SetCanEverAffectNavigation(false);
	
	DetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &UGrapplePointDetectorComponent::OnOverlapStart);
	DetectionVolume->OnComponentEndOverlap.AddDynamic(this, &UGrapplePointDetectorComponent::OnOverlapEnd);
}

void UGrapplePointDetectorComponent::GetOverlappedGrapplePoints()
{
	TArray<AActor*> OverlappingActors;
	DetectionVolume->GetOverlappingActors(OverlappingActors, AGrapplePoint::StaticClass());

	for (int i = 0; i < OverlappingActors.Num(); i++)
		AvailableGrapplePoints.Add(Cast<AGrapplePoint>(OverlappingActors[i]));
}

void UGrapplePointDetectorComponent::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Add(GP);
}

void UGrapplePointDetectorComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Remove(GP);
}


TArray<AGrapplePoint*>* UGrapplePointDetectorComponent::GetAvailableGrapplePoints()
{
	return &AvailableGrapplePoints;
}
