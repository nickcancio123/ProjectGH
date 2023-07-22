// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrappleThrustComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h" 
#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Components/CommonGrappleComponent.h"

#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"




#pragma region Default Actor Component Functions
UGrappleThrustComponent::UGrappleThrustComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGrappleThrustComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();
}
#pragma endregion



#pragma region Initializer Functions
void UGrappleThrustComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("GrappleThrust", IE_Pressed,this, &UGrappleThrustComponent::TryGrappleThrust);
}

#pragma endregion



#pragma region Grappling Driver Functions
void UGrappleThrustComponent::TryGrappleThrust()
{
	if (!CommonGrappleComp->CanGrapple())
		return;

	AGrapplePoint* BestValidGrapplePoint = CommonGrappleComp->GetBestValidGrapplePoint();
	if (BestValidGrapplePoint)
	{
		CommonGrappleComp->SetCurrentGrapplePoint(BestValidGrapplePoint);
		StartGrappleSequence();
		return;
	}

	CommonGrappleComp->SetCurrentGrapplePoint(nullptr);
}

void UGrappleThrustComponent::StartGrappleSequence()
{
	CommonGrappleComp->SetCanGrapple(false);
	CommonGrappleComp->SetCurrentGrappleType(EGrappleType::GT_Thrust);
	
	GrappleThrustState = EGrappleThrustState::GTS_Throw;
	
	UAnimMontage* ThrowMontage = CharacterMovement->IsFalling() ? GrappleThrowAirMontage : GrappleThrowMontage;
	Character->PlayAnimMontage(ThrowMontage);
}

void UGrappleThrustComponent::StartGrappleThrust()
{	
	Character->PlayAnimMontage(GrappleThrustMontage);
}

void UGrappleThrustComponent::ReleaseGrapple()
{
	CommonGrappleComp->SetCanGrapple(true);
	CommonGrappleComp->SetCurrentGrappleType(EGrappleType::GT_None);
	
	GrappleThrustState = EGrappleThrustState::GTS_Idle;
	  	
	// Set rotation 
	FVector HorizVel = CharacterMovement->Velocity;
	HorizVel.Z = 0;
	Character->SetActorRotation(HorizVel.Rotation());
	CharacterMovement->bOrientRotationToMovement = true;
}
#pragma endregion



#pragma region Setters & Getters
void UGrappleThrustComponent::SetCommonGrappleComp(UCommonGrappleComponent* _CommonGrappleComp)
{
	CommonGrappleComp = _CommonGrappleComp;
}

void UGrappleThrustComponent::SetGrappleThrustState(EGrappleThrustState _GrappleThrustState)
{
	GrappleThrustState = _GrappleThrustState;
}
#pragma endregion

EGrappleThrustState UGrappleThrustComponent::GetGrappleThrustState()
{
	return GrappleThrustState;
}
#pragma endregion
