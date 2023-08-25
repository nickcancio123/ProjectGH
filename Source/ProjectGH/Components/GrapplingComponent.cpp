// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrapplingComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Actors/GrapplingHook.h"

#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"



#pragma region Actor Component Methods
UGrapplingComponent::UGrapplingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GrapplePointDetectionVolume = CreateDefaultSubobject<USphereComponent>("Detection Volume");
}

void UGrapplingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());
	
	GetOverlappedGrapplePoints();
	CreateGrapplingHookActor();
}

void UGrapplingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsActive())
		return;

	FindBestValidGrapplePoint();
}

void UGrapplingComponent::OnRegister()
{
	Super::OnRegister();

	InitDetectionVolume();
}
#pragma endregion



#pragma region Common Grappling Methods
void UGrapplingComponent::InitDetectionVolume()
{
	GrapplePointDetectionVolume->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GrapplePointDetectionVolume->SetRelativeLocation(FVector::ZeroVector);
	
	GrapplePointDetectionVolume->SetSphereRadius(GrappleRange.Y);
	GrapplePointDetectionVolume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	GrapplePointDetectionVolume->SetCanEverAffectNavigation(false);
	
	GrapplePointDetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &UGrapplingComponent::OnOverlapStart);
	GrapplePointDetectionVolume->OnComponentEndOverlap.AddDynamic(this, &UGrapplingComponent::OnOverlapEnd);
}

void UGrapplingComponent::GetOverlappedGrapplePoints()
{
	TArray<AActor*> OverlappingActors;
	GrapplePointDetectionVolume->GetOverlappingActors(OverlappingActors, AGrapplePoint::StaticClass());

	for (int i = 0; i < OverlappingActors.Num(); i++)
		AvailableGrapplePoints.Add(Cast<AGrapplePoint>(OverlappingActors[i]));
}

void UGrapplingComponent::CreateGrapplingHookActor()
{
	GrapplingHook = Cast<AGrapplingHook>(GetWorld()->SpawnActor(GrapplingHookClass));
	if (!GrapplingHook)
		return;

	GrapplingHook->SetupGrapplingHook(Character->GetMesh());
	GrapplingHook->SetGrapplingHookState(EGrapplingHookState::GHS_In);
}


void UGrapplingComponent::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Add(GP);
}

void UGrapplingComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Remove(GP);
}


void UGrapplingComponent::FindBestValidGrapplePoint()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	Character->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector LineOfSight = ViewRotation.Vector().GetSafeNormal();

	float MinGPAngle = 361;
	AGrapplePoint* BestGrapplePointYet = nullptr;

	// Check each available GP if valid to grapple to, if so keep track of best option
	
	for (int i = 0; i < AvailableGrapplePoints.Num(); i++)
	{
		AGrapplePoint* GrapplePoint = AvailableGrapplePoints[i];
		
		// Get normalized vector from view location to GP
		FVector CamToGP =  GrapplePoint->GetActorLocation() - ViewLocation;
		float DistToGP = CamToGP.Size();
		CamToGP.Normalize();
		
		bool bValidDist = DistToGP > GrappleRange.X;
		if (!bValidDist)
			continue;

		// Check if angle is valid
		float Angle = FMath::Acos(FVector::DotProduct(LineOfSight, CamToGP));
		Angle = FMath::RadiansToDegrees(Angle);
		
		bool bValidAngle = Angle < MaxAimAngleToGrapple;
		if (!bValidAngle)
			continue;

		// Raycast to see if line of sight is obstructed to GP
		FHitResult HitResult;
		
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Character);
		CollisionParams.AddIgnoredActor(GrapplePoint);
		
		bool bTraceHit = GetWorld()->LineTraceSingleByProfile(
			HitResult,
			ViewLocation,
			GrapplePoint->GetActorLocation(),
			"BlockAll",
			CollisionParams
		);
		
		if (!bTraceHit)
		{
			if (Angle < MinGPAngle)
			{
				MinGPAngle = Angle;
				BestGrapplePointYet = GrapplePoint;	
			}
		}
	}
	
	BestValidGrapplePoint = BestGrapplePointYet ? BestGrapplePointYet : nullptr;
}
#pragma endregion 



#pragma region Swinging


#pragma endregion



#pragma region Thrusting


#pragma endregion 

