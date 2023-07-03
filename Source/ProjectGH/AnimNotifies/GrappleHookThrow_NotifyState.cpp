// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleHookThrow_NotifyState.h"

#include "ProjectGH/Actors/Hero.h"
#include "ProjectGH/Components/CommonGrappleComponent.h"
#include "ProjectGH/Components/GrappleThrustComponent.h"
#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Actors/GrapplePoint.h"


void UGrappleHookThrow_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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
	
	HandPos = MeshComp->GetSocketLocation("RightHandSocket");
	GP_Pos = GrapplePoint->GetActorLocation();
}

void UGrappleHookThrow_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!GrapplingHook || !GrapplingHook->HookMeshComp || !GrapplePoint)
		return;

	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / NotifyTotalDuration;
	
	// If GP moves, recompute path every frame
	if (GrapplePoint->bMoves)
		GP_Pos = GrapplePoint->GetActorLocation();
	
	// Set hook position
	FVector NewHookPos = FMath::Lerp(HandPos, GP_Pos, Alpha);
	GrapplingHook->HookMeshComp->SetWorldLocation(NewHookPos);
}

void UGrappleHookThrow_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!GrapplingHook || !GrappleThrustComp)
		return;

	GrappleThrustComp->SetGrappleThrustState(EGrappleThrustState::GTS_Thrust);
}