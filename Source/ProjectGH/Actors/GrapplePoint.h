// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplePoint.generated.h"


UCLASS()
class PROJECTGH_API AGrapplePoint : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grapple Point")
	USceneComponent* Root = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Point")
		UStaticMeshComponent* MeshComp = nullptr;
	
	// Whether or not this GP moves. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Point")
		bool bMoves = false;
	
	AGrapplePoint();

	virtual void Tick(float DeltaTime) override;

	UStaticMesh* GetMesh();

protected:
	virtual void BeginPlay() override;
};

