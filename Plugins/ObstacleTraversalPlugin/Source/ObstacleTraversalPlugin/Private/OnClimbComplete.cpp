// Fill out your copyright notice in the Description page of Project Settings.


#include "OnClimbComplete.h"
#include "ObstacleTraversalComponent.h"

#include "GameFramework/Character.h"


void UOnClimbComplete::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	ObstacleTraversalComp = Cast<UObstacleTraversalComponent>(Character->GetComponentByClass(UObstacleTraversalComponent::StaticClass()));
	if (ObstacleTraversalComp)
		ObstacleTraversalComp->OnClimbComplete();
}