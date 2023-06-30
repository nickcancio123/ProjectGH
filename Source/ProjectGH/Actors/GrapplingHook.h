// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplingHook.generated.h"


class UCableComponent;
class UGrappleComponent;


UCLASS()
class PROJECTGH_API AGrapplingHook : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Hook")
		UCableComponent* CableComp = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Hook")
		UStaticMeshComponent* HookMeshComp = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Hook")
		float CableWidth = 5;
	
	AGrapplingHook();

	virtual void Tick(float DeltaTime) override;


	void SetupCable(USkeletalMeshComponent* CharacterMesh);
	void SetVisibility(bool bVisible);
	void SetGrappleComp(UGrappleComponent* _GrappleComp);

	
protected:
	virtual void BeginPlay() override;


private:
	UGrappleComponent* GrappleComp = nullptr;
	
	void InitHookMesh();
};
