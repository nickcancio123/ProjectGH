// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrapplingHook.h"

#include "DrawDebugHelpers.h"
#include "Viewports.h"
#include "ProjectGH/Actors/GrapplePoint.h" 
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"


UGrapplingHook::UGrapplingHook()
{
	PrimaryComponentTick.bCanEverTick = true;

	InitGrapplePointDetector();
}

void UGrapplingHook::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());

	GetOverlapped_GPs();
}

void UGrapplingHook::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGrapplingHook::OnRegister()
{
	Super::OnRegister();

	GP_Detector->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}



void UGrapplingHook::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Grapple", IE_Pressed,this, &UGrapplingHook::TryGrapple);
}

void UGrapplingHook::SetCanGrapple(bool _bCanGrapple)
{
	bCanGrapple = _bCanGrapple;
}

void UGrapplingHook::InitGrapplePointDetector()
{
	GP_Detector = CreateDefaultSubobject<USphereComponent>(TEXT("Grapple Point Detector"));
	
	GP_Detector->SetSphereRadius(GrappleRange.GetUpperBoundValue());
	GP_Detector->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	GP_Detector->SetCanEverAffectNavigation(false);

	GP_Detector->OnComponentBeginOverlap.AddDynamic(this, &UGrapplingHook::OnOverlapStart);
	GP_Detector->OnComponentEndOverlap.AddDynamic(this, &UGrapplingHook::OnOverlapEnd);
}

// Called on BeginPlay to check for already overlapped GPs
void UGrapplingHook::GetOverlapped_GPs()
{
	TArray<AActor*> OverlappingActors;
	GP_Detector->GetOverlappingActors(OverlappingActors, AGrapplePoint::StaticClass());

	for (int i = 0; i < OverlappingActors.Num(); i++)
		Available_GPs.Add(Cast<AGrapplePoint>(OverlappingActors[i]));
}



void UGrapplingHook::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		Available_GPs.Add(GP);
}

void UGrapplingHook::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		Available_GPs.Remove(GP);
}




void UGrapplingHook::TryGrapple()
{
	if (!bCanGrapple)
		return;
	
	FVector ViewLocation;
	FRotator ViewRotation;
	Character->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector LineOfSight = ViewRotation.Vector().GetSafeNormal();

	bool bFoundValidGP = false;
	float MinGPAngle = 361;
	int BestGPIndex = 0;
	
	for (int i = 0; i < Available_GPs.Num(); i++)
	{
		// Get normalized vector from view location to GP
		FVector CamToGP =  (Available_GPs[i]->GetActorLocation() - ViewLocation);
		float DistToGP = CamToGP.Size();
		CamToGP.Normalize();

		float Angle = FMath::Acos(FVector::DotProduct(LineOfSight, CamToGP));
		Angle = FMath::RadiansToDegrees(Angle);

		if (Angle < Max_GP_SightAngle && DistToGP > GrappleRange.GetLowerBoundValue())
		{
			bFoundValidGP = true;
			if (Angle < MinGPAngle)
			{
				MinGPAngle = Angle;
				BestGPIndex = i;	
			}
		}
	}

	if (bFoundValidGP)
	{
		Current_GP = Available_GPs[BestGPIndex];
		BeginGrapple();
	}
}

void UGrapplingHook::BeginGrapple()
{
	DrawDebugSphere(
	GetWorld(),
	Current_GP->GetActorLocation(),
	43,
	8,
	FColor::Yellow,
	false,
	2
	);

	bCanGrapple = false;
	
	Character->PlayAnimMontage(GrappleAnimMontage);
}

// Returns the grapple point that is actively being grappled to
AGrapplePoint* UGrapplingHook::GetCurrentGrapplePoint()
{
	return Current_GP;
}

FVector UGrapplingHook::GetGrappleDirection()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	Character->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector CamToGP =  (Current_GP->GetActorLocation() - ViewLocation).GetSafeNormal();
	return CamToGP;
}
