// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleHookPull_Notify.h"

#include "ProjectGH/Components/GrapplingComponent.h"
#include "ProjectGH/Actors/GrapplingHook.h"

#include "GameFramework/Character.h"


void UGrappleHookPull_Notify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
		return;
	
	UGrapplingComponent* GrapplingComp = Cast<UGrapplingComponent>(Character->GetComponentByClass(UGrapplingComponent::StaticClass()));
	GrapplingComp->GetGrapplingHook()->SetGrapplingHookState(EGrapplingHookState::GHS_Pull);
}
