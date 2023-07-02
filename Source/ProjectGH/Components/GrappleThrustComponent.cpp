// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrappleThrustComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h" 
#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Components/CommonGrappleComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"




#pragma region Default Actor Component Functions
UGrappleThrustComponent::UGrappleThrustComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrappleThrustComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();

	CommonGrappleComp = Cast<UCommonGrappleComponent>(Character->GetComponentByClass(UCommonGrappleComponent::StaticClass()));
	
	CreateGrappleHookActor();
}

void UGrappleThrustComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FindBestValidGP();

	// If hanging, clamp velocity and position
	if (GrappleThrustState == EGrappleThrustState::GTS_Hang)
		HangTick(DeltaTime);
}
#pragma endregion



#pragma region Initializer Functions
void UGrappleThrustComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("GrappleThrust", IE_Pressed,this, &UGrappleThrustComponent::TryGrappleThrust);
	PlayerInputComponent->BindAction("GrappleThrust", IE_Released,this, &UGrappleThrustComponent::ReleaseGrappleInput);
}

void UGrappleThrustComponent::CreateGrappleHookActor()
{
	GrapplingHook = Cast<AGrapplingHook>(GetWorld()->SpawnActor(GrapplingHookClass));
	if (!GrapplingHook)
		return;

	GrapplingHook->SetGrappleThrustComp(this);
	GrapplingHook->SetupCable(Character->GetMesh());
	GrapplingHook->SetVisibility(false);
}
#pragma endregion



#pragma region Grappling Driver Functions
void UGrappleThrustComponent::FindBestValidGP()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	Character->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector LineOfSight = ViewRotation.Vector().GetSafeNormal();

	float MinGPAngle = 361;
	AGrapplePoint* BestGrapplePointYet = nullptr;

	// Check each available GP if valid to grapple to, if so keep track of best option
	TArray<AGrapplePoint*>* AvailableGrapplePoints = CommonGrappleComp->GetAvailableGrapplePoints();
	
	for (int i = 0; i < AvailableGrapplePoints->Num(); i++)
	{
		AGrapplePoint* GrapplePoint = (*AvailableGrapplePoints)[i];
		
		// Get normalized vector from view location to GP
		FVector CamToGP =  GrapplePoint->GetActorLocation() - ViewLocation;
		float DistToGP = CamToGP.Size();
		CamToGP.Normalize();
		
		bool bValidDist = DistToGP > GrappleThrustRange.GetLowerBoundValue();
		if (!bValidDist)
			continue;

		// Check if angle is valid
		float Angle = FMath::Acos(FVector::DotProduct(LineOfSight, CamToGP));
		Angle = FMath::RadiansToDegrees(Angle);
		
		bool bValidAngle = Angle < MaxGrappleAimAngle;
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
	
	BestValid_GP = BestGrapplePointYet ? BestGrapplePointYet : nullptr;
}

void UGrappleThrustComponent::TryGrappleThrust()
{
	bHoldingInput = true;
	
	if (!bCanGrapple)
		return;
	
	if (BestValid_GP)
	{
		Current_GP = BestValid_GP;
		BeginGrappleThrust();
	}
	else
	{
		Current_GP = nullptr;
	}
}

void UGrappleThrustComponent::BeginGrappleThrust()
{
	bCanGrapple = false;
	GrappleThrustState = EGrappleThrustState::GTS_Throw;
	Character->PlayAnimMontage(GrappleAnimMontage);
}


void UGrappleThrustComponent::ReleaseGrappleInput()
{
	bHoldingInput = false;

	if (GrappleThrustState == EGrappleThrustState::GTS_Hang)
		Character->PlayAnimMontage(HangDismountMontage);		
}

void UGrappleThrustComponent::ReleaseGrapple()
{
	GrapplingHook->SetVisibility(false);
	GrappleThrustState = EGrappleThrustState::GTS_Idle;
	bCanGrapple = true;
	  	
	// Set rotation settings
	FVector HorizVel = CharacterMovement->Velocity;
	HorizVel.Z = 0;
	Character->SetActorRotation(HorizVel.Rotation());
	CharacterMovement->bOrientRotationToMovement = true;
}


void UGrappleThrustComponent::HangTick(float DeltaTime)
{
	FVector GP_Pos = Current_GP->GetActorLocation();
	FVector HeroPos = Character->GetActorLocation();

	
	// Set new velocity
	FVector Vel = CharacterMovement->Velocity;
	FVector HeroToGP = GP_Pos - HeroPos;
	float Dist = HeroToGP.Size();
	HeroToGP.Normalize();

	FVector NewVel = Vel;
	if (Dist >= GrappleHangDist)
	{
		FVector NewPos = Current_GP->GetActorLocation() + (-GrappleHangDist * HeroToGP);
		Character->SetActorLocation(NewPos);

		NewVel = FVector::VectorPlaneProject(Vel, HeroToGP);
		CharacterMovement->Velocity = NewVel;
	}

	
	// Set new rotation 
	FRotator TargetRot = UKismetMathLibrary::MakeRotFromXZ(CharacterMovement->Velocity, HeroToGP);
	
	FRotator NewRot = UKismetMathLibrary::RInterpTo(
		Character->GetActorRotation(),
		TargetRot,
		DeltaTime,
		HangRotationRate
		);

	Character->SetActorRotation(NewRot);

	
	// Change states if grounded
	if (!CharacterMovement->IsFalling())
		ReleaseGrapple();
}
#pragma endregion



#pragma region Setters
void UGrappleThrustComponent::SetCanGrapple(bool _bCanGrapple)
{
	bCanGrapple = _bCanGrapple;
}

void UGrappleThrustComponent::SetGrappleState(EGrappleThrustState _GrappleThrustState)
{
	GrappleThrustState = _GrappleThrustState;
}
#pragma endregion



#pragma region Accessors
AGrapplePoint* UGrappleThrustComponent::GetBestValidGrapplePoint()
{
	return BestValid_GP;
}

EGrappleThrustState UGrappleThrustComponent::GetGrappleThrustState()
{
	return GrappleThrustState;
}

AGrapplePoint* UGrappleThrustComponent::GetCurrentGrapplePoint()
{
	return Current_GP;
}

AGrapplingHook* UGrappleThrustComponent::GetGrapplingHook()
{
	return GrapplingHook;
}

FVector UGrappleThrustComponent::GetGrappleDirection()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	Character->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector CamToGP =  (Current_GP->GetActorLocation() - ViewLocation).GetSafeNormal();
	return CamToGP;
}

bool UGrappleThrustComponent::IsHoldingInput()
{
	return bHoldingInput;
}
#pragma endregion
