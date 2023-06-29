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
	GP = GrappleComp->GetCurrentGrapplePoint();
	
	GrapplingHook = GrappleComp->GetGrapplingHook();
	GrapplingHook->ReleaseHookFromActor();
	
	HandPos = MeshComp->GetSocketLocation("RightHandSocket");
	GP_Pos = GP->GetActorLocation();
}

void UGrappleHook_Pull_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!GrapplingHook || !GrapplingHook->HookMeshComp || !GP)
		return;
	
	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / NotifyTotalDuration;
	
	if (Alpha > PullBackPercent)
		GrapplingHook->SetVisibility(false);
	
	// If GP moves, recompute path every frame
	if (GP->bMoves)
		GP_Pos = GP->GetActorLocation();
	
	// Set hook position
	FVector NewHookPos = FMath::Lerp(HandPos, GP_Pos, Alpha);
	GrapplingHook->HookMeshComp->SetWorldLocation(NewHookPos);
}

void UGrappleHook_Pull_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!GrapplingHook)
		return;
	
	GrapplingHook->SetVisibility(false);
}