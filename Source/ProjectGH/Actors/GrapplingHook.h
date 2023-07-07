// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplingHook.generated.h"


class UCableComponent;
class UCommonGrappleComponent;

class UGrappleThrustComponent;


UENUM()
enum EGrapplingHookState
{
	GHS_In = 0,		// not in use, not visible
	GHS_Throw = 1,	// going out, handled by notify state
	GHS_Out = 2,	// in use, attached to something, visible
	GHS_Pull = 3	// reeling back in
};

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Hook")
		float PullInDuration = 0.2f;
	
	AGrapplingHook();

	virtual void Tick(float DeltaTime) override;


	void SetupCable(USkeletalMeshComponent* CharacterMesh);
	void SetCommonGrappleComp(UCommonGrappleComponent* _CommonGrappleComp);
	
	void SetVisibility(bool bVisible);
	void SetHookRotationToCableDir();

	void SetGrapplingHookState(EGrapplingHookState State);
	
protected:
	virtual void BeginPlay() override;


private:
	UCommonGrappleComponent* CommonGrappleComp = nullptr;
	UGrappleThrustComponent* GrappleThrustComp = nullptr;

	EGrapplingHookState GrapplingHookState = EGrapplingHookState::GHS_In;
	float StateTimer = 0;

	void OutStateTick();
	void PullStateTick(float DeltaTime);
};
