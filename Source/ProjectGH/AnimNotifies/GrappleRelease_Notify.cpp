// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleRelease_Notify.h"
#include "ProjectGH/Components/GrappleThrustComponent.h"
#include "ProjectGH/Actors/Hero.h"



void UGrappleRelease_Notify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	Hero = Cast<AHero>(MeshComp->GetOwner());
	if (!Hero)
		return;
	
	GrappleThrustComp = Cast<UGrappleThrustComponent>(Hero->GetComponentByClass(UGrappleThrustComponent::StaticClass()));
	GrappleThrustComp->ReleaseGrapple();
}
