// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleHook_Pull_NotifyState.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Actors/Hero.h"
#include "ProjectGH/Components/GrappleComponent.h"
#include "ProjectGH/Actors/GrapplingHook.h"



void UGrappleHook_Pull_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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
	
	PathDir = HandPos - GP_Pos;
	TotalPathDist = PathDir.Size();
	PathDir.Normalize();
}

void UGrappleHook_Pull_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!GrapplingHook || !GrapplingHook->HookMeshComp)
		return;

	
	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / NotifyTotalDuration;


	if (Alpha > PullBackPercent)
		GrapplingHook->SetVisibility(false);
	
	
	// Set hook position
	FVector NewHookPos = GP_Pos + (Alpha * TotalPathDist * PathDir);
	GrapplingHook->HookMeshComp->SetWorldLocation(NewHookPos);

	// Set hook rotation
	GrapplingHook->HookMeshComp->SetWorldRotation(PathDir.Rotation());
}

void UGrappleHook_Pull_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!GrapplingHook)
		return;
	
	GrapplingHook->SetVisibility(false);
}