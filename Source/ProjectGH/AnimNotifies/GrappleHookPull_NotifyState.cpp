// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleHookPull_NotifyState.h"

#include "ProjectGH/Actors/Hero.h"

#include "ProjectGH/Components/CommonGrappleComponent.h"
#include "ProjectGH/Components/GrappleThrustComponent.h"
#include "ProjectGH/Components/GrappleSwingComponent.h"

#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Actors/GrapplePoint.h"



void UGrappleHookPull_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											 float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	NotifyTotalDuration = TotalDuration;
	RunningTime = 0;

	Hero = Cast<AHero>(MeshComp->GetOwner());
	if (!Hero)
		return;

	CommonGrappleComp = Cast<UCommonGrappleComponent>(Hero->GetComponentByClass(UCommonGrappleComponent::StaticClass()));
	GrapplePoint = CommonGrappleComp->GetCurrentGrapplePoint();

	GrapplingHook = CommonGrappleComp->GetGrapplingHook();
	GrapplingHook->SetVisibility(true);
	
	GrappleThrustComp = Cast<UGrappleThrustComponent>(Hero->GetComponentByClass(UGrappleThrustComponent::StaticClass()));
	GrappleSwingComp = Cast<UGrappleSwingComponent>(Hero->GetComponentByClass(UGrappleSwingComponent::StaticClass()));
	
	GP_Pos = GrapplePoint->GetActorLocation();
}

void UGrappleHookPull_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!GrapplingHook || !GrapplingHook->HookMeshComp || !GrapplePoint)
		return;

	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / NotifyTotalDuration;

	if (Alpha > MaxLerpAlpha)
		return;
	
	// If GP moves, recompute path every frame
	if (GrapplePoint->bMoves)
		GP_Pos = GrapplePoint->GetActorLocation();

	HandPos = MeshComp->GetSocketLocation("RightHandSocket");

	// Set hook position
	FVector NewHookPos = FMath::Lerp(GP_Pos, HandPos, Alpha);
	GrapplingHook->HookMeshComp->SetWorldLocation(NewHookPos);
	GrapplingHook->SetHookRotationToCableDir();
}

void UGrappleHookPull_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	
	//if (GrapplingHook)
	//{
	//	GrapplingHook->SetHookActive(false);
	//	GrapplingHook->SetVisibility(false);
	//}
}