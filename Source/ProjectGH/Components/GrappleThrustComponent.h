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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
	 	UClass* GrapplingHookClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		UAnimMontage* GrappleAnimMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		UAnimMontage* HangDismountMontage = nullptr;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		FFloatRange GrappleThrustRange = FFloatRange(700, 3000);
	
	// The max angle (degrees) between line-of-sight and vector to GP to consider for grappling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		float MaxGrappleAimAngle = 20;

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
	void SetCanGrapple(bool _bCanGrapple);
	void SetGrappleState(EGrappleThrustState _GrappleThrustState);

	// Getters
	UFUNCTION(BlueprintCallable)
	AGrapplePoint* GetBestValidGrapplePoint();

	UFUNCTION(BlueprintCallable)
	EGrappleThrustState GetGrappleThrustState();
	
	AGrapplingHook* GetGrapplingHook();
	AGrapplePoint* GetCurrentGrapplePoint();
	FVector GetGrappleDirection();
	bool IsHoldingInput();


	
protected:
	virtual void BeginPlay() override;

	

private:
	ACharacter* Character = nullptr;
	UCharacterMovementComponent* CharacterMovement = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;
	UCommonGrappleComponent* CommonGrappleComp = nullptr;
	
	// This frame's best valid GP option
	AGrapplePoint* BestValid_GP = nullptr;

	// GP currently begin grappled to 
	AGrapplePoint* Current_GP = nullptr;

	EGrappleThrustState GrappleThrustState = EGrappleThrustState::GTS_Idle;
	bool bCanGrapple = true;
	bool bHoldingInput = false;


	
	// Initializers
	void CreateGrappleHookActor();

	
	// Grapple driver functions
	void TryGrappleThrust();
	void FindBestValidGP();
	void BeginGrappleThrust();
	void HangTick(float DeltaTime);
};
