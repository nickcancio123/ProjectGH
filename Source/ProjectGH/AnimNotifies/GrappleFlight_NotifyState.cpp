// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleFlight_NotifyState.h"

#include "ProjectGH/Components/GrappleThrustComponent.h"
#include "ProjectGH/Actors/Hero.h"
#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Actors/GrapplePoint.h"
#include "GameFramework/PawnMovementComponent.h"

#include "DrawDebugHelpers.h"

void UGrappleFlight_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	
	Hero = Cast<AHero>(MeshComp->GetOwner());
	if (!Hero)
		return;
	
	GrappleThrustComp = Cast<UGrappleThrustComponent>(Hero->GetComponentByClass(UGrappleThrustComponent::StaticClass()));
	GrapplePoint = GrappleThrustComp->GetCurrentGrapplePoint();

	SpringArm = Cast<USpringArmComponent>(Hero->GetComponentByClass(USpringArmComponent::StaticClass()));
	OriginalSpringArmLength = SpringArm->TargetArmLength;

	TotalNotifyDuration = TotalDuration;
	RunningTime = 0;

	PathStart = Hero->GetActorLocation();
	PathEnd = GrapplePoint->GetActorLocation();
}


void UGrappleFlight_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!Hero || !SpringArm)
		return;

	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / TotalNotifyDuration;

	
	// Compute and set new actor location
	if (GrapplePoint->bMoves)
		PathEnd = GrapplePoint->GetActorLocation();

	FVector NewLocation = FMath::Lerp(PathStart, PathEnd, Alpha);
	NewLocation.Z += PathShapeCurve.GetRichCurve()->Eval(Alpha) * PathHeightScale;
	Hero->SetActorLocation(NewLocation);

	
	// Compute new spring arm distance
	SpringArm->TargetArmLength = OriginalSpringArmLength + SpringArmLengthCurve.GetRichCurve()->Eval(Alpha) * 1000;
}


void UGrappleFlight_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!Hero || !GrappleThrustComp)
		return;

	// Set post grapple velocity
	float PathTotalDist = FVector::Dist(PathStart, PathEnd);
	FVector PathDir = (PathEnd - PathStart).GetSafeNormal();
	
	float GrappleSpeed = PathTotalDist / TotalNotifyDuration;
	FVector PostGrappleVelocity = GrappleSpeed * PercentSpeedRetainedPostGrapple * PathDir;
	
	FVector MoveInput = Hero->GetMoveInput();
	FVector WorldMoveInput =
			(MoveInput.X * Hero->GetControlForwardVector()) +
				(MoveInput.Y * Hero->GetControlRightVector());
	
	PostGrappleVelocity += GrappleSpeed * PercentSpeedInputSpeed * WorldMoveInput;
	
	Hero->GetMovementComponent()->Velocity = PostGrappleVelocity;


	// Change state
	if (GrappleThrustComp->IsHoldingInput())
	{
		GrappleThrustComp->SetGrappleState(EGrappleThrustState::GTS_Hang);
		Hero->GetCharacterMovement()->bOrientRotationToMovement = false;
	}	
	else
	{
		GrappleThrustComp->ReleaseGrapple();
	}
}