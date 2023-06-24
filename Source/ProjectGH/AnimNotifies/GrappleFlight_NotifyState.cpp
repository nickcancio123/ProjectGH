// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleFlight_NotifyState.h"

#include "DrawDebugHelpers.h"
#include "ProjectGH/Components/GrapplingHook.h"
#include "GameFramework/Pawn.h"
#include "ProjectGH/Actors/GrapplePoint.h"
#include "GameFramework/PawnMovementComponent.h"

void UGrappleFlight_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	
	Pawn = Cast<APawn>(MeshComp->GetOwner());

	if (!Pawn)
		return;
	
	GrapplingHook = Cast<UGrapplingHook>(Pawn->GetComponentByClass(UGrapplingHook::StaticClass()));
	GrapplePoint = GrapplingHook->GetCurrentGrapplePoint();

	SpringArm = Cast<USpringArmComponent>(Pawn->GetComponentByClass(USpringArmComponent::StaticClass()));
	OriginalSpringArmLength = SpringArm->TargetArmLength;

	TotalNotifyDuration = TotalDuration;
	RunningTime = 0;

	PathStart = Pawn->GetActorLocation();
	PathEnd = GrapplePoint->GetActorLocation();

	PathTotalDist = FVector::Dist(PathStart, PathEnd);
	PathDir = (PathEnd - PathStart).GetSafeNormal();
}

void UGrappleFlight_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!Pawn)
		return;

	RunningTime += FrameDeltaTime;

	float Alpha = RunningTime / TotalNotifyDuration;

	// Compute and set new actor location
	float DistAlongPath = Alpha * PathTotalDist;
	FVector NewLocation = PathStart + DistAlongPath * PathDir;
	NewLocation.Z += PathShapeCurve.GetRichCurve()->Eval(Alpha) * PathHeightScale;
	Pawn->SetActorLocation(NewLocation);

	// Compute new spring arm distance
	SpringArm->TargetArmLength = OriginalSpringArmLength + SpringArmLengthCurve.GetRichCurve()->Eval(Alpha) * 1000;
}

void UGrappleFlight_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!Pawn)
		return;

	GrapplingHook->SetCanGrapple(true);
	
	Pawn->GetMovementComponent()->Velocity = PathDir * PostGrappleVelocity;
}