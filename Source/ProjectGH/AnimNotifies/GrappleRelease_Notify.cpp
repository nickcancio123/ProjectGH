// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleRelease_Notify.h"

#include "ProjectGH/Components/GrapplingComponent.h"
#include "GameFramework/Character.h"



void UGrappleRelease_Notify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	GrapplingComp = Cast<UGrapplingComponent>(Character->GetComponentByClass(UGrapplingComponent::StaticClass()));
	GrapplingComp->ReleaseGrappleFromSwing();
}
