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
}

void UGrappleThrustComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	//if (GrappleThrustState == EGrappleThrustState::GTS_Hang)
	//	HangTick(DeltaTime);
}
#pragma endregion



#pragma region Initializer Functions
void UGrappleThrustComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("GrappleThrust", IE_Pressed,this, &UGrappleThrustComponent::TryGrappleThrust);
	PlayerInputComponent->BindAction("GrappleThrust", IE_Released,this, &UGrappleThrustComponent::ReleaseGrappleInput);
}

#pragma endregion



#pragma region Grappling Driver Functions
void UGrappleThrustComponent::TryGrappleThrust()
{
	bHoldingInput = true;
	
	if (!CommonGrappleComp->CanGrapple())
		return;

	AGrapplePoint* BestValidGrapplePoint = CommonGrappleComp->GetBestValidGrapplePoint();
	if (BestValidGrapplePoint)
	{
		CommonGrappleComp->SetCurrentGrapplePoint(BestValidGrapplePoint);
		BeginGrappleThrust();
		return;
	}

	CommonGrappleComp->SetCurrentGrapplePoint(nullptr);
}

void UGrappleThrustComponent::BeginGrappleThrust()
{
	CommonGrappleComp->SetCanGrapple(false);
	CommonGrappleComp->SetCurrentGrappleType(EGrappleType::GT_Thrust);
	
	GrappleThrustState = EGrappleThrustState::GTS_Throw;
	
	Character->PlayAnimMontage(GrappleThrustMontage);
}


void UGrappleThrustComponent::ReleaseGrappleInput()
{
	bHoldingInput = false;
}

void UGrappleThrustComponent::ReleaseGrapple()
{
	CommonGrappleComp->SetCanGrapple(true);
	CommonGrappleComp->SetCurrentGrappleType(EGrappleType::GT_None);
	CommonGrappleComp->GetGrapplingHook()->SetGrapplingHookState(EGrapplingHookState::GHS_Pull);
	
	GrappleThrustState = EGrappleThrustState::GTS_Idle;
	  	
	// Set rotation 
	FVector HorizVel = CharacterMovement->Velocity;
	HorizVel.Z = 0;
	Character->SetActorRotation(HorizVel.Rotation());
	CharacterMovement->bOrientRotationToMovement = true;
}


void UGrappleThrustComponent::HangTick(float DeltaTime)
{
	FVector GP_Pos = CommonGrappleComp->GetCurrentGrapplePoint()->GetActorLocation();
	FVector HeroPos = Character->GetActorLocation();

	
	// Set new velocity
	FVector Vel = CharacterMovement->Velocity;
	FVector HeroToGP = GP_Pos - HeroPos;
	float Dist = HeroToGP.Size();
	HeroToGP.Normalize();

	FVector NewVel = Vel;
	if (Dist >= GrappleHangDist)
	{
		FVector NewPos = GP_Pos + (-GrappleHangDist * HeroToGP);
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
void UGrappleThrustComponent::SetGrappleThrustState(EGrappleThrustState _GrappleThrustState)
{
	GrappleThrustState = _GrappleThrustState;
}
#pragma endregion



#pragma region Accessors
EGrappleThrustState UGrappleThrustComponent::GetGrappleThrustState()
{
	return GrappleThrustState;
}

bool UGrappleThrustComponent::IsHoldingInput()
{
	return bHoldingInput;
}
#pragma endregion
