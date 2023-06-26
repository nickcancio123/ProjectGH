// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/GrapplingHook.h"
#include "CableComponent.h"
#include "NavigationSystemTypes.h"



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
	CableComp->CableWidth = CableWidth;
	CableComp->bEnableStiffness = true;
	CableComp->SolverIterations = 6;
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
	CableComp->SetActive(bVisible);
	
	HookMeshComp->SetVisibility(bVisible);
}



