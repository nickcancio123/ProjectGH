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
	
	InitHookMesh();
}

void AGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
}

void AGrapplingHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	
	if (!CommonGrappleComp || !CommonGrappleComp->GetCurrentGrapplePoint())
		return;
	
	// If currently grappling, attach grapple to current grapple point
	//if (GrappleThrustComp->GetGrappleThrustState() == EGrappleThrustState::GTS_Thrust ||
	//	GrappleThrustComp->GetGrappleThrustState() == EGrappleThrustState::GTS_Hang)
	
	if (bHookActive)
	{
		HookMeshComp->SetWorldLocation(CommonGrappleComp->GetCurrentGrapplePoint()->GetActorLocation());
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
	CableComp->bSkipCableUpdateWhenNotVisible = true;
	CableComp->CableWidth = CableWidth;
	CableComp->bEnableStiffness = true;
	CableComp->SolverIterations = 5;
}

void AGrapplingHook::InitHookMesh()
{
	HookMeshComp->SetRelativeScale3D(FVector::OneVector * 0.2);
}




void AGrapplingHook::SetVisibility(bool bVisible)
{
	if (!CableComp || !HookMeshComp)
		return;
	
	CableComp->SetVisibility(bVisible);
	HookMeshComp->SetVisibility(bVisible);
}

void AGrapplingHook::SetHookActive(bool _bActive)
{
	bHookActive = _bActive;
}

void AGrapplingHook::SetCommonGrappleComp(UCommonGrappleComponent* _CommonGrappleComp)
{
	CommonGrappleComp = _CommonGrappleComp;
}



