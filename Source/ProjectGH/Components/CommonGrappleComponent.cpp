// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/CommonGrappleComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "Components/SphereComponent.h"



#pragma region Default Actor Component Methods
UCommonGrappleComponent::UCommonGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	DetectionVolume = CreateDefaultSubobject<USphereComponent>("Detection Volume");
}

void UCommonGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOverlappedGrapplePoints();
}

void UCommonGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCommonGrappleComponent::OnRegister()
{
	Super::OnRegister();
	InitDetectionVolume();
}
#pragma endregion


void UCommonGrappleComponent::InitDetectionVolume()
{
	DetectionVolume->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	DetectionVolume->SetRelativeLocation(FVector::ZeroVector);
	
	DetectionVolume->SetSphereRadius(DetectionRadius);
	DetectionVolume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	DetectionVolume->SetCanEverAffectNavigation(false);
	
	DetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &UCommonGrappleComponent::OnOverlapStart);
	DetectionVolume->OnComponentEndOverlap.AddDynamic(this, &UCommonGrappleComponent::OnOverlapEnd);
}

void UCommonGrappleComponent::GetOverlappedGrapplePoints()
{
	TArray<AActor*> OverlappingActors;
	DetectionVolume->GetOverlappingActors(OverlappingActors, AGrapplePoint::StaticClass());

	for (int i = 0; i < OverlappingActors.Num(); i++)
		AvailableGrapplePoints.Add(Cast<AGrapplePoint>(OverlappingActors[i]));
}

void UCommonGrappleComponent::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Add(GP);
}

void UCommonGrappleComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Remove(GP);
}


TArray<AGrapplePoint*>* UCommonGrappleComponent::GetAvailableGrapplePoints()
{
	return &AvailableGrapplePoints;
}
