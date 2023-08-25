// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleThrust_NotifyState.h"

//#include "ProjectGH/Components/CommonGrappleComponent.h"
//#include "ProjectGH/Components/GrappleThrustComponent.h"
#include "ProjectGH/Components/GrapplingComponent.h"

#include "ProjectGH/Actors/GrapplingHook.h"
#include "ProjectGH/Actors/GrapplePoint.h"

#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Character.h"



void UGrappleThrust_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	
	Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	
	//CommonGrappleComp = Cast<UCommonGrappleComponent>(Character->GetComponentByClass(UCommonGrappleComponent::StaticClass()));
	//
	//GrapplingHook = CommonGrappleComp->GetGrapplingHook();
	//GrapplingHook->SetGrapplingHookState(EGrapplingHookState::GHS_Out);
	//
	//GrapplePoint = CommonGrappleComp->GetCurrentGrapplePoint();
	//
	//GrappleThrustComp = Cast<UGrappleThrustComponent>(Character->GetComponentByClass(UGrappleThrustComponent::StaticClass()));
	//GrappleThrustComp->SetGrappleThrustState(EGrappleThrustState::GTS_Thrust);


	GrapplingComp = Cast<UGrapplingComponent>(Character->GetComponentByClass(UGrapplingComponent::StaticClass()));
	GrapplingComp->SetGrappleThrustPhase(GTP_Thrust);
	
	GrapplingHook = GrapplingComp->GetGrapplingHook();
	GrapplingHook->SetGrapplingHookState(GHS_Out);

	GrapplePoint = GrapplingComp->GetCurrentGrapplePoint();
	
	
	SpringArm = Cast<USpringArmComponent>(Character->GetComponentByClass(USpringArmComponent::StaticClass()));
	OriginalSpringArmLength = SpringArm->TargetArmLength;

	TotalNotifyDuration = TotalDuration;
	RunningTime = 0;

	PathStart = Character->GetActorLocation();
	PathEnd = GrapplePoint->GetActorLocation();
}


void UGrappleThrust_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!Character || !SpringArm)
		return;

	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / TotalNotifyDuration;

	
	// Compute and set new actor location
	if (GrapplePoint->bMoves)
		PathEnd = GrapplePoint->GetActorLocation();

	FVector NewLocation = FMath::Lerp(PathStart, PathEnd, Alpha);
	NewLocation.Z += PathShapeCurve.GetRichCurve()->Eval(Alpha) * PathHeightScale;
	Character->SetActorLocation(NewLocation);

	
	// Compute new spring arm distance
	SpringArm->TargetArmLength = OriginalSpringArmLength + SpringArmLengthCurve.GetRichCurve()->Eval(Alpha) * 1000;


	// Animate grapple icon
	//CommonGrappleComp->SpinGrappleIcon(FrameDeltaTime);
	GrapplingComp->SpinGrappleIcon(FrameDeltaTime);
}


void UGrappleThrust_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	//if (!Character || !GrappleThrustComp || !CommonGrappleComp)
	//	return;
	if (!Character && !GrapplingComp)
		return;

	//CommonGrappleComp->ResetGrappleIconAngle();
	GrapplingComp->ResetGrappleIconAngle();

	// Set post grapple velocity
	float PathTotalDist = FVector::Dist(PathStart, PathEnd);
	FVector PathDir = (PathEnd - PathStart).GetSafeNormal();
	
	float GrappleSpeed = PathTotalDist / TotalNotifyDuration;
	FVector PostGrappleVelocity = GrappleSpeed * PostThrustSpeedRetained * PathDir;
	
	UInputComponent* InputComp = Character->InputComponent;
	if (!InputComp)
		return;

		// Get move input
	float ForwardInput = InputComp->GetAxisValue("MoveForward");
	float RightInput = InputComp->GetAxisValue("MoveRight");
	FVector MoveInput = (ForwardInput * FVector::ForwardVector) + (RightInput * FVector::RightVector);
	MoveInput.Normalize();

		// Get control rotation vectors
	FVector ControlForward = FVector::VectorPlaneProject(Character->GetControlRotation().Vector(), FVector::UpVector);
	FVector ControlRight = FVector::CrossProduct(FVector::UpVector, Character->GetControlRotation().Vector());
	ControlForward.Normalize();
	ControlRight.Normalize();

		// Compute and set character world velocity
	FVector WorldMoveInput = (MoveInput.X * ControlForward) + (MoveInput.Y * ControlRight);
	PostGrappleVelocity += GrappleSpeed * PostThrustInputImpulseSpeed * WorldMoveInput;
	Character->GetMovementComponent()->Velocity = PostGrappleVelocity;

	
	// Finish grapple thrust
	GrapplingHook->SetGrapplingHookState(GHS_In);
	//GrappleThrustComp->ReleaseGrapple();
	GrapplingComp->ReleaseGrapple();

}