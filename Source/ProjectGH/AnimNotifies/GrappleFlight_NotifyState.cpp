// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleFlight_NotifyState.h"

#include "DrawDebugHelpers.h"
#include "ProjectGH/Components/GrappleComponent.h"
#include "ProjectGH/Actors/Hero.h"
#include "ProjectGH/Actors/GrapplePoint.h"
#include "GameFramework/PawnMovementComponent.h"

void UGrappleFlight_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	
	Hero = Cast<AHero>(MeshComp->GetOwner());

	if (!Hero)
		return;
	
	GrappleComp = Cast<UGrappleComponent>(Hero->GetComponentByClass(UGrappleComponent::StaticClass()));
	GrapplePoint = GrappleComp->GetCurrentGrapplePoint();

	SpringArm = Cast<USpringArmComponent>(Hero->GetComponentByClass(USpringArmComponent::StaticClass()));
	OriginalSpringArmLength = SpringArm->TargetArmLength;

	TotalNotifyDuration = TotalDuration;
	RunningTime = 0;

	PathStart = Hero->GetActorLocation();
	PathEnd = GrapplePoint->GetActorLocation();

	PathTotalDist = FVector::Dist(PathStart, PathEnd);
	PathDir = (PathEnd - PathStart).GetSafeNormal();
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
	float DistAlongPath = Alpha * PathTotalDist;
	FVector NewLocation = PathStart + DistAlongPath * PathDir;
	NewLocation.Z += PathShapeCurve.GetRichCurve()->Eval(Alpha) * PathHeightScale;
	Hero->SetActorLocation(NewLocation);

	// Compute new spring arm distance
	SpringArm->TargetArmLength = OriginalSpringArmLength + SpringArmLengthCurve.GetRichCurve()->Eval(Alpha) * 1000;
}

void UGrappleFlight_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!Hero)
		return;

	
	float GrappleSpeed = PathTotalDist / TotalNotifyDuration;
	FVector PostGrappleVelocity = GrappleSpeed * PercentSpeedRetainedPostGrapple * PathDir;
	
	FVector MoveInput = Hero->GetMoveInput();
	FVector WorldMoveInput =
			(MoveInput.X * Hero->GetControlForwardVector()) +
				(MoveInput.Y * Hero->GetControlRightVector());
	
	PostGrappleVelocity += GrappleSpeed * PercentSpeedInputSpeed * WorldMoveInput;

	Hero->GetMovementComponent()->Velocity = PostGrappleVelocity;


	
	GrappleComp->SetCanGrapple(true);
}