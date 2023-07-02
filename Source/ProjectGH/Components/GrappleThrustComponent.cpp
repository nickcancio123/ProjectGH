// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrappleThrustComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h" 
#include "ProjectGH/Actors/GrapplingHook.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"


#include "CableComponent.h"



#pragma region Default Actor Component Functions
void UGrappleThrustComponent::OnRegister()
{
	Super::OnRegister();

	GP_Detector->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GP_Detector->SetRelativeLocation(FVector::ZeroVector);

	GP_Detector->OnComponentBeginOverlap.AddDynamic(this, &UGrappleThrustComponent::OnOverlapStart);
	GP_Detector->OnComponentEndOverlap.AddDynamic(this, &UGrappleThrustComponent::OnOverlapEnd);
}

UGrappleThrustComponent::UGrappleThrustComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	InitGrapplePointDetector();
}

void UGrappleThrustComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();
	
	CreateGrappleHookActor();
	GetOverlapped_GPs();
}

void UGrappleThrustComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FindBestValidGP();

	// If hanging, clamp velocity and position
	if (GrappleState == EGrappleState::Hang)
		HangTick(DeltaTime);
	
}
#pragma endregion



#pragma region Initializer Functions
void UGrappleThrustComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Grapple", IE_Pressed,this, &UGrappleThrustComponent::TryGrapple);
	PlayerInputComponent->BindAction("Grapple", IE_Released,this, &UGrappleThrustComponent::ReleaseGrappleInput);
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

void UGrappleThrustComponent::InitGrapplePointDetector()
{
	GP_Detector = CreateDefaultSubobject<USphereComponent>(TEXT("Grapple Point Detector"));
	
	GP_Detector->SetSphereRadius(GrappleRange.GetUpperBoundValue());
	GP_Detector->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	GP_Detector->SetCanEverAffectNavigation(false);
}

void UGrappleThrustComponent::GetOverlapped_GPs()
{
	TArray<AActor*> OverlappingActors;
	GP_Detector->GetOverlappingActors(OverlappingActors, AGrapplePoint::StaticClass());

	for (int i = 0; i < OverlappingActors.Num(); i++)
		Available_GPs.Add(Cast<AGrapplePoint>(OverlappingActors[i]));
}
#pragma endregion



#pragma region Grappling Driver Functions
void UGrappleThrustComponent::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		Available_GPs.Add(GP);
}

void UGrappleThrustComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		Available_GPs.Remove(GP);
}

void UGrappleThrustComponent::TryGrapple()
{
	bHoldingInput = true;
	
	if (!bCanGrapple)
		return;
	
	if (BestValid_GP)
	{
		Current_GP = BestValid_GP;
		BeginGrapple();
	}
	else
	{
		Current_GP = nullptr;
	}
}

void UGrappleThrustComponent::FindBestValidGP()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	Character->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector LineOfSight = ViewRotation.Vector().GetSafeNormal();

	bool bFoundValidGP = false;
	float MinGPAngle = 361;
	int BestGPIndex = 0;

	// Check each available GP if valid to grapple to, if so keep track of best option
	for (int i = 0; i < Available_GPs.Num(); i++)
	{
		// Get normalized vector from view location to GP
		FVector CamToGP =  (Available_GPs[i]->GetActorLocation() - ViewLocation);
		float DistToGP = CamToGP.Size();
		CamToGP.Normalize();
		
		bool bValidDist = DistToGP > GrappleRange.GetLowerBoundValue();
		if (!bValidDist)
			continue;

		// Check if angle is valid
		float Angle = FMath::Acos(FVector::DotProduct(LineOfSight, CamToGP));
		Angle = FMath::RadiansToDegrees(Angle);
		
		bool bValidAngle = Angle < Max_GP_SightAngle;
		if (!bValidAngle)
			continue;

		// Raycast to see if line of sight is obstructed to GP
		FHitResult HitResult;
		
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Character);
		CollisionParams.AddIgnoredActor(Available_GPs[i]);
		
		bool bTraceHit = GetWorld()->LineTraceSingleByProfile(
			HitResult,
			ViewLocation,
			Available_GPs[i]->GetActorLocation(),
			"BlockAll",
			CollisionParams
		);
		
		if (!bTraceHit)
		{
			bFoundValidGP = true;
			if (Angle < MinGPAngle)
			{
				MinGPAngle = Angle;
				BestGPIndex = i;	
			}
		}
	}
	
	BestValid_GP = bFoundValidGP ? Available_GPs[BestGPIndex] : nullptr;
}

void UGrappleThrustComponent::BeginGrapple()
{
	bCanGrapple = false;
	GrappleState = EGrappleState::Throw;
	Character->PlayAnimMontage(GrappleAnimMontage);
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

void UGrappleThrustComponent::ReleaseGrappleInput()
{
	bHoldingInput = false;

	if (GrappleState == EGrappleState::Hang)
		Character->PlayAnimMontage(HangDismountMontage);		
}

void UGrappleThrustComponent::ReleaseGrapple()
{
	GrapplingHook->SetVisibility(false);
	GrappleState = EGrappleState::Idle;
	bCanGrapple = true;
	  	
	// Set rotation settings
	FVector HorizVel = CharacterMovement->Velocity;
	HorizVel.Z = 0;
	Character->SetActorRotation(HorizVel.Rotation());
	CharacterMovement->bOrientRotationToMovement = true;
}
#pragma endregion



#pragma region Setters
void UGrappleThrustComponent::SetCanGrapple(bool _bCanGrapple)
{
	bCanGrapple = _bCanGrapple;
}

void UGrappleThrustComponent::SetGrappleState(EGrappleState _GrappleState)
{
	GrappleState = _GrappleState;
}
#pragma endregion



#pragma region Accessors
AGrapplePoint* UGrappleThrustComponent::GetBestValidGrapplePoint()
{
	return BestValid_GP;
}

EGrappleState UGrappleThrustComponent::GetGrappleState()
{
	return GrappleState;
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
