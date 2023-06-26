// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleHookThrow_NotifyState.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Actors/Hero.h"
#include "ProjectGH/Components/GrappleComponent.h"
#include "ProjectGH/Actors/GrapplingHook.h"


void UGrappleHookThrow_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											 float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	NotifyTotalDuration = TotalDuration;
	RunningTime = 0;

	Hero = Cast<AHero>(MeshComp->GetOwner());
	if (!Hero)
		return;

	GrappleComp = Cast<UGrappleComponent>(Hero->GetComponentByClass(UGrappleComponent::StaticClass()));
	GrapplingHook = GrappleComp->GetGrapplingHook();
	GrapplingHook->SetVisibility(true);
	
	
	HandPos = MeshComp->GetSocketLocation("RightHandSocket");
	GP_Pos = GrappleComp->GetCurrentGrapplePoint()->GetActorLocation();
	
	PathDir = GP_Pos - HandPos;
	TotalPathDist = PathDir.Size();
	PathDir.Normalize();
}

void UGrappleHookThrow_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!GrapplingHook || !GrapplingHook->HookMeshComp)
		return;

	
	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / NotifyTotalDuration;

	
	// Set hook position
	FVector NewHookPos = HandPos + (Alpha * TotalPathDist * PathDir);
	GrapplingHook->HookMeshComp->SetWorldLocation(NewHookPos);
}

void UGrappleHookThrow_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
}