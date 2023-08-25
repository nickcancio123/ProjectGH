// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/RotateTowardsTarget_NotifyState.h"

//#include "ProjectGH/Components/CommonGrappleComponent.h"
#include "ProjectGH/Components/GrapplingComponent.h"

#include "GameFramework/Pawn.h"
#include "ProjectGH/Actors/GrapplePoint.h"

void URotateTowardsTarget_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                   float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);


	NotifyTotalDuration = TotalDuration;
	RunningTime = 0;

	Pawn = Cast<APawn>(MeshComp->GetOwner());
	if (!Pawn)
		return;
	
	StartRot = Pawn->GetActorRotation();


	switch (TargetType)
	{
	case ETargetType::ViewDirection:
		{
			FVector ControlLookDir = Pawn->GetControlRotation().Vector();
			ControlLookDir.Z = 0;
			TargetRot = ControlLookDir.Rotation();
			break;
		}
	case ETargetType::GrapplePoint:
		{
			//UCommonGrappleComponent* CommonGrappleComp = 
			//	Cast<UCommonGrappleComponent>(Pawn->GetComponentByClass(UCommonGrappleComponent::StaticClass()));
			//if (CommonGrappleComp)
			//{
			//	FVector GrappleDir = CommonGrappleComp->GetCurrentGrapplePoint()->GetActorLocation() - Pawn->GetActorLocation();
			//	GrappleDir.Z = 0;
			//	TargetRot = GrappleDir.Rotation();
			//}

			UGrapplingComponent* GrapplingComp = Cast<UGrapplingComponent>(Pawn->GetComponentByClass(UGrapplingComponent::StaticClass()));
			if (GrapplingComp)
			{
				FVector GrappleDir = GrapplingComp->GetCurrentGrapplePoint()->GetActorLocation() - Pawn->GetActorLocation();
				GrappleDir.Z = 0;
				TargetRot = GrappleDir.Rotation();
			}
			break;
		}
	default:
		TargetRot = FRotator::ZeroRotator;
	}
}

void URotateTowardsTarget_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!Pawn)
		return;
	
	RunningTime += FrameDeltaTime;
	float Alpha = RunningTime / NotifyTotalDuration;

	FRotator NewRot = FMath::Lerp(StartRot, TargetRot, Alpha);
	Pawn->SetActorRotation(NewRot);
}

void URotateTowardsTarget_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
}