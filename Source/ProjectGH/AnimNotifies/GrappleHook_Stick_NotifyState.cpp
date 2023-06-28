// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleHook_Stick_NotifyState.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Actors/Hero.h"
#include "ProjectGH/Components/GrappleComponent.h"
#include "ProjectGH/Actors/GrapplingHook.h"


void UGrappleHook_Stick_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											 float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	
	Hero = Cast<AHero>(MeshComp->GetOwner());
	if (!Hero)
		return;

	GrappleComp = Cast<UGrappleComponent>(Hero->GetComponentByClass(UGrappleComponent::StaticClass()));
	GP = GrappleComp->GetCurrentGrapplePoint();
	GrapplingHook = GrappleComp->GetGrapplingHook();
}

void UGrappleHook_Stick_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!GrapplingHook || !GrapplingHook->HookMeshComp || !GP)
		return;
	
	if (!GP->bMoves)
		return;
	
	GrapplingHook->HookMeshComp->SetWorldLocation(GP->GetActorLocation());
}

void UGrappleHook_Stick_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
}