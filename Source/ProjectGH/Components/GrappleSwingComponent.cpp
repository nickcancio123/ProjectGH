// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrappleSwingComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Components/CommonGrappleComponent.h"

#include "GameFramework/Character.h"




#pragma region Default Actor Component Methods
UGrappleSwingComponent::UGrappleSwingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrappleSwingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();

	CommonGrappleComp = Cast<UCommonGrappleComponent>(Character->GetComponentByClass(UCommonGrappleComponent::StaticClass()));
}

void UGrappleSwingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
#pragma endregion



#pragma region Initializer Functions
void UGrappleSwingComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("GrappleSwing", IE_Pressed,this, &UGrappleSwingComponent::TryGrappleSwing);
	PlayerInputComponent->BindAction("GrappleSwing", IE_Released,this, &UGrappleSwingComponent::ReleaseGrappleInput);
}
#pragma endregion 



#pragma region Grappling Driver Functions
void UGrappleSwingComponent::TryGrappleSwing()
{

}

void UGrappleSwingComponent::ReleaseGrappleInput()
{
}
#pragma endregion 
