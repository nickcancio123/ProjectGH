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

	SetupCable();
}

void AGrapplingHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TEST
	// Time += DeltaTime;
	// FVector Pos = 400 * FMath::Sin(Time) * FVector::UpVector;
	// Pos += 600 * FVector::UpVector;
	// SetHookWorldLocation(Pos);
}

void AGrapplingHook::SetHookWorldLocation(FVector Location)
{
	HookMeshComp->SetWorldLocation(Location);
}


void AGrapplingHook::SetupCable()
{
	CableComp->bAttachEnd = true;
	CableComp->EndLocation = FVector::ZeroVector;
	CableComp->SetAttachEndToComponent(HookMeshComp);
	
	CableComp->CableWidth = CableWidth;
}

void AGrapplingHook::InitHookMesh()
{
	HookMeshComp->SetRelativeScale3D(FVector::OneVector * 0.3);
}

