// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/GrapplingHook.h"

#include "ProjectGH//Components/CommonGrappleComponent.h"
#include "ProjectGH/Components/GrappleThrustComponent.h"
#include "ProjectGH/Actors/GrapplePoint.h"
#include "CableComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"



AGrapplingHook::AGrapplingHook()
{
	PrimaryActorTick.bCanEverTick = true;

	CableComp = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	SetRootComponent(CableComp);
	
	HookMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hook Mesh"));
}

void AGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
}

void AGrapplingHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (GrapplingHookState)
	{
	case EGrapplingHookState::GHS_In: HookMeshComp->SetWorldLocation(CableComp->GetComponentLocation()); break;
	case EGrapplingHookState::GHS_Throw: break;
	case EGrapplingHookState::GHS_Out: OutStateTick(); break;
	case EGrapplingHookState::GHS_Pull: PullStateTick(DeltaTime); break;
	}
}



void AGrapplingHook::SetupCable(USkeletalMeshComponent* CharacterMesh)
{
	// Attach end to hook mesh
	CableComp->bAttachEnd = true;
	CableComp->EndLocation = FVector::ZeroVector;
	CableComp->SetAttachEndToComponent(HookMeshComp);

	// Attach start to character right hand socket
	CableComp->bAttachStart = true;
	CableComp->AttachToComponent(CharacterMesh, FAttachmentTransformRules::KeepRelativeTransform,"RightHandSocket");
	CableComp->SetRelativeLocation(FVector::ZeroVector);

	// Misc
	CableComp->SetTickGroup(ETickingGroup::TG_PostPhysics);
	CableComp->bSkipCableUpdateWhenNotVisible = false;
	CableComp->CableWidth = CableWidth;
	CableComp->bEnableStiffness = false;
	CableComp->SolverIterations = 4;
}

void AGrapplingHook::SetCommonGrappleComp(UCommonGrappleComponent* _CommonGrappleComp)
{
	CommonGrappleComp = _CommonGrappleComp;
}



void AGrapplingHook::OutStateTick()
{
	// Set hook location
	HookMeshComp->SetWorldLocation(CommonGrappleComp->GetCurrentGrapplePoint()->GetActorLocation());

	// Set hook rotation
	SetHookRotationToCableDir();	
}

void AGrapplingHook::PullStateTick(float DeltaTime)
{
	StateTimer += DeltaTime;
	float Alpha = StateTimer / PullInDuration;


	// State end condition
	if (Alpha >= 1)
	{
		SetGrapplingHookState(GHS_In);
	}
	
	
	if (!CommonGrappleComp)
		return;

	AGrapplePoint* CurrentGrapplePoint = CommonGrappleComp->GetCurrentGrapplePoint();
	if (!CurrentGrapplePoint)
		return;
	
	FVector GrapplePointPos = CurrentGrapplePoint->GetActorLocation();
	FVector HandPos = CableComp->GetComponentLocation();

	// Set hook position
	FVector NewHookPos = FMath::Lerp(GrapplePointPos, HandPos, Alpha);
	HookMeshComp->SetWorldLocation(NewHookPos);
	SetHookRotationToCableDir();
}



void AGrapplingHook::SetVisibility(bool bVisible)
{
	if (!CableComp || !HookMeshComp)
		return;
	
	CableComp->SetVisibility(bVisible);
	HookMeshComp->SetVisibility(bVisible);
}

void AGrapplingHook::SetHookRotationToCableDir()
{
	FVector CableDir = GetActorLocation() - HookMeshComp->GetComponentLocation();
	HookMeshComp->SetWorldRotation(CableDir.Rotation());
}

void AGrapplingHook::SetGrapplingHookState(EGrapplingHookState State)
{
	GrapplingHookState = State;
	StateTimer = 0;

	switch (GrapplingHookState)
	{
	case EGrapplingHookState::GHS_In: SetVisibility(false); break;
	case EGrapplingHookState::GHS_Throw: SetVisibility(true); break;
	case EGrapplingHookState::GHS_Out: SetVisibility(true); break;
	case EGrapplingHookState::GHS_Pull: break;
	}
}

