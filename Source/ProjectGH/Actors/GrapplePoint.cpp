// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Actors/GrapplePoint.h"

AGrapplePoint::AGrapplePoint()
{
	PrimaryActorTick.bCanEverTick = true;


	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);


	// Create and init mesh
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	MeshComp->SetupAttachment(GetRootComponent());
	MeshComp->SetCollisionProfileName("OverlapAllDynamic");
}

void AGrapplePoint::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGrapplePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UStaticMesh* AGrapplePoint::GetMesh()
{
	if (!MeshComp)
		return nullptr;
	
	return MeshComp->GetStaticMesh();
}

