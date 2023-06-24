// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/AnimNotifies/GrappleFlight_NotifyState.h"

#include "DrawDebugHelpers.h"
#include "ProjectGH/Components/GrapplingHook.h"
#include "GameFramework/Character.h"
#include "ProjectGH/Actors/GrapplePoint.h"

void UGrappleFlight_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	
	Character = Cast<ACharacter>(MeshComp->GetOwner());

	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing character reference in grapple notify state"));
		return;
	}
	
	GrapplingHook = Cast<UGrapplingHook>(Character->GetComponentByClass(UGrapplingHook::StaticClass()));
	GrapplePoint = GrapplingHook->GetCurrentGrapplePoint();


	TotalNotifyDuration = TotalDuration;
	RunningTime = 0;

	PathStart = Character->GetActorLocation();
	PathEnd = GrapplePoint->GetActorLocation();

	PathTotalDist = FVector::Dist(PathStart, PathEnd);
	PathDir = (PathEnd - PathStart).GetSafeNormal();
}

void UGrappleFlight_NotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (!Character)
		return;

	RunningTime += FrameDeltaTime;

	float Alpha = RunningTime / TotalNotifyDuration;
	
	float DistAlongPath = Alpha * PathTotalDist;
	FVector NewLocation = PathStart + DistAlongPath * PathDir;
	NewLocation.Z += PathShape.GetRichCurve()->Eval(Alpha) * PathHeightScale;
	
	Character->SetActorLocation(NewLocation);
}

void UGrappleFlight_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
}