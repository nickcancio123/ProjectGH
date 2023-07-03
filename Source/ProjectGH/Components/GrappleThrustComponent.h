// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GrappleThrustComponent.generated.h"


class AGrapplePoint;
class AGrapplingHook;
class USphereComponent;
class UInputComponent;
class UAnimMontage;

class UCommonGrappleComponent;



UENUM(BlueprintType)
enum EGrappleThrustState
{
	GTS_Idle = 0,
	GTS_Throw = 1,
	GTS_Thrust = 2,
	GTS_Hang = 3
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTGH_API UGrappleThrustComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
	//  	UClass* GrapplingHookClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		UAnimMontage* GrappleThrustMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		UAnimMontage* HangDismountMontage = nullptr;

	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		float GrappleHangDist = 600;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		float HangRotationRate = 8;

	
	
	UGrappleThrustComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	void ReleaseGrappleInput();
	void ReleaseGrapple();
	

	// Setters
	void BindInput(UInputComponent* PlayerInputComponent);
	void SetGrappleThrustState(EGrappleThrustState _GrappleThrustState);

	// Getters
	UFUNCTION(BlueprintCallable)
	EGrappleThrustState GetGrappleThrustState();
	
	AGrapplingHook* GetGrapplingHook();
	bool IsHoldingInput();


	
protected:
	virtual void BeginPlay() override;

	

private:
	ACharacter* Character = nullptr;
	UCharacterMovementComponent* CharacterMovement = nullptr;
	UCommonGrappleComponent* CommonGrappleComp = nullptr;
	//AGrapplingHook* GrapplingHook = nullptr;

	EGrappleThrustState GrappleThrustState = EGrappleThrustState::GTS_Idle;
	bool bHoldingInput = false;


	
	// Initializers
	//void CreateGrappleHookActor();

	
	// Grapple driver functions
	void TryGrappleThrust();
	void BeginGrappleThrust();
	void HangTick(float DeltaTime);
};
