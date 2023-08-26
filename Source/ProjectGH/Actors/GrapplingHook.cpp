// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/GrapplingHook.h"

#include "ProjectGH/Components/GrapplingComponent.h"
#include "ProjectGH/Actors/GrapplePoint.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CableComponent.h"



AGrapplingHook::AGrapplingHook()
{
	PrimaryActorTick.bCanEverTick = true;

	CableComp = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	SetRootComponent(CableComp);
	
	HookMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hook Mesh"));

	SplineMeshComp = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Spline Mesh"));
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
	case EGrapplingHookState::GHS_In:
		HookMeshComp->SetWorldLocation(CableComp->GetComponentLocation());
		break;
		
	case EGrapplingHookState::GHS_Throw:
		{
			UpdateSplineMesh();
			break;
		}
		
	case EGrapplingHookState::GHS_Out:
		OutStateTick();
		break;
		
	case EGrapplingHookState::GHS_Pull:
		PullStateTick(DeltaTime);
		break;
	}
}


void AGrapplingHook::SetupGrapplingHook(USkeletalMeshComponent* _CharacterMesh)
{
	CharacterMesh = _CharacterMesh;
	
	// ===== Cable stuff =====
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


	// ===== Spline stuff =====
	SplineMeshComp->SetMobility(EComponentMobility::Movable);
	SplineMeshComp->SetForwardAxis(ESplineMeshAxis::X);
}


void AGrapplingHook::OutStateTick()
{
	AGrapplePoint* GrapplePoint = GrapplingComp->GetCurrentGrapplePoint();
	if (!GrapplePoint)
		return;
	FVector GrapplePointPos = GrapplePoint->GetActorLocation();

	
	// Set hook location
	HookMeshComp->SetWorldLocation(GrapplePointPos);

	// Set hook rotation
	SetHookRotationToCableDir();
	

	UpdateSplineMesh();
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
	
	if (!GrapplingComp)
		return;

	AGrapplePoint* CurrentGrapplePoint = GrapplingComp->GetCurrentGrapplePoint();
	
	if (!CurrentGrapplePoint)
		return;
	
	FVector GrapplePointPos = CurrentGrapplePoint->GetActorLocation();
	FVector HandPos = CableComp->GetComponentLocation();

	// Set hook position
	FVector NewHookPos = FMath::Lerp(GrapplePointPos, HandPos, Alpha);
	HookMeshComp->SetWorldLocation(NewHookPos);
	SetHookRotationToCableDir();
}


void AGrapplingHook::SetGrapplingCompRef(UGrapplingComponent* _GrapplingComp)
{
	GrapplingComp = _GrapplingComp;
}

void AGrapplingHook::SetHookRotationToCableDir()
{
	FVector CableDir = GetActorLocation() - HookMeshComp->GetComponentLocation();
	HookMeshComp->SetWorldRotation(CableDir.Rotation());
}

void AGrapplingHook::UpdateSplineMesh()
{
	SplineMeshComp->SetWorldLocation(CharacterMesh->GetSocketLocation("RightHandSocket"));
	
	FVector TargetDir = HookMeshComp->GetComponentLocation() - GetActorLocation();
	SplineMeshComp->SetWorldRotation(TargetDir.Rotation());
	SplineMeshComp->SetEndPosition(FVector::ForwardVector * TargetDir.Size());
}

void AGrapplingHook::SetGrapplingHookState(EGrapplingHookState State)
{
	GrapplingHookState = State;
	StateTimer = 0;

	switch (GrapplingHookState)
	{
	case EGrapplingHookState::GHS_In:
		CableComp->SetVisibility(false);
		HookMeshComp->SetVisibility(false);
		SplineMeshComp->SetVisibility(false);
		break;
		
	case EGrapplingHookState::GHS_Throw:
		CableComp->SetVisibility(true);
		HookMeshComp->SetVisibility(true);
		SplineMeshComp->SetVisibility(false);
		break;
		
	case EGrapplingHookState::GHS_Out:
		CableComp->SetVisibility(false);
		HookMeshComp->SetVisibility(true);
		SplineMeshComp->SetVisibility(true);
		break;
		
	case EGrapplingHookState::GHS_Pull:
		CableComp->SetVisibility(true);
		HookMeshComp->SetVisibility(true);
		SplineMeshComp->SetVisibility(false);
		break;
	}
}

EGrapplingHookState AGrapplingHook::GetGrapplingHookState()
{
	return GrapplingHookState;
}

