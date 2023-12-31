// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleHookThrow_NotifyState.h"

#include "ProjectGH/Components/GrapplingComponent.h"
#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Actors/GrapplePoint.h"

#include "GameFramework/Character.h"


void UGrappleHookThrow_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											 float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	NotifyTotalDuration = TotalDuration;
	RunningTime = 0;

	Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	GrapplingComp = Cast<UGrapplingComponent>(Character->GetComponentByClass(UGrapplingComponent::StaticClass()));
	GrapplePoint = GrapplingComp->GetCurrentGrapplePoint();

	GrapplingHook = GrapplingComp->GetGrapplingHook();
	GrapplingHook->SetGrapplingHookState(GHS_Throw);

	
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
	GrapplingHook->SetHookRotationToCableDir();
}

void UGrappleHookThrow_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	
	if (GrapplingHook)
		GrapplingHook->SetGrapplingHookState(EGrapplingHookState::GHS_Out);	

	if (!GrapplingComp)
		return;

	EGrappleState CurrentGrappleState = GrapplingComp->GetCurrentGrappleState();
	switch (CurrentGrappleState)
	{
	case EGrappleState::GS_None:
		break;
		
	case EGrappleState::GS_Swing:
		{
			if (GrapplingComp)
				GrapplingComp->StartSwingPhase();
			break;
		}
		
	case EGrappleState::GS_Thrust:
		{
			if (GrapplingComp)
				GrapplingComp->StartGrappleThrust();
			break;
		}
	}
}