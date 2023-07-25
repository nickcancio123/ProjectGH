// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GrappleSwingComponent.generated.h"


class AGrapplePoint;
class AGrapplingHook;
class UCommonGrappleComponent;
class UBoxComponent;

class UInputComponent;
class UAnimMontage;



UENUM(BlueprintType)
enum EGrappleSwingState
{
	GSS_Idle = 0,
	GSS_Throw = 1,
	GSS_Swing = 3
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTGH_API UGrappleSwingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		UAnimMontage* GrappleThrowMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		UAnimMontage* GrappleThrowAirMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		UAnimMontage* SwingDismountMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		UAnimMontage* KipUpMontage = nullptr;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		FVector GroundDetectorVolumeExtent = FVector(25, 50, 100);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		float MaxHorizAngleToKipUp = 55;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		float SwingRotationRate = 8;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		float MaxSwingRotationRate = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		bool bReleaseGrappleOnGrounded = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Swing")
		float MinPercentDistanceNeededToDismount = 0.5f;

	
	UGrappleSwingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnRegister() override;
	
	void BindInput(UInputComponent* PlayerInputComponent);
	void SetGrappleSwingState(EGrappleSwingState _GrappleSwingState);

	void StartSwingState();
	void ReleaseGrapple();

	UFUNCTION(BlueprintCallable)
	EGrappleSwingState GetGrappleSwingState();

	UFUNCTION(BlueprintCallable)
	void SetCommonGrappleComp(UCommonGrappleComponent* _CommonGrappleComp);
	
protected:
	virtual void BeginPlay() override;


private:
	ACharacter* Character = nullptr;
	UCharacterMovementComponent* CharacterMovement = nullptr;
	UCommonGrappleComponent* CommonGrappleComp = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;
	UBoxComponent* GroundDetectionVolume = nullptr;
	
	EGrappleSwingState GrappleSwingState = EGrappleSwingState::GSS_Idle;

	bool bHoldingInput = false;
	float InitSwingDist = 0;
	bool bCanSwingWhileOnGround = false;
	FVector LastFrameVelocity = FVector::ZeroVector;
	
	// Swing driver methods
	void TryGrappleSwing();
	void BeginSwingSequence();
	void SwingStateTick(float DeltaTime);
	void ReleaseGrappleInput();

	UFUNCTION()
	void OnGroundOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	// Misc
	void InitGroundDetectorVolume();
	void SetSwingEndActorRotation();
	bool CanDoAnimatedDismount();
};
