// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GrapplingHook.generated.h"


class UCableComponent;
class UGrapplingComponent;


UENUM(BlueprintType)
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
		USplineMeshComponent* SplineMeshComp = nullptr;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Hook")
		float CableWidth = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Hook")
		float PullInDuration = 0.2f;
	
	AGrapplingHook();

	virtual void Tick(float DeltaTime) override;


	void SetupGrapplingHook(USkeletalMeshComponent* _CharacterMesh);
	void SetGrapplingCompRef(UGrapplingComponent* _GrapplingComp);
	
	void SetHookRotationToCableDir();
	void SetGrapplingHookState(EGrapplingHookState State);

	UFUNCTION(BlueprintCallable)
	EGrapplingHookState GetGrapplingHookState();
	
protected:
	virtual void BeginPlay() override;


private:
	UGrapplingComponent* GrapplingComp = nullptr;
	USkeletalMeshComponent* CharacterMesh = nullptr;

	EGrapplingHookState GrapplingHookState = EGrapplingHookState::GHS_In;
	float StateTimer = 0;

	void OutStateTick();
	void PullStateTick(float DeltaTime);
	
	void UpdateSplineMesh();
};
