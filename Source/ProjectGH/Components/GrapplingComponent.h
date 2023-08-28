// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapplingComponent.generated.h"


class UBoxComponent;
class UCharacterMovementComponent;
class AGrapplePoint;
class AGrapplingHook;
class USphereComponent;



UENUM(BlueprintType)
enum EGrappleState
{
	GS_None = 0,
	GS_Swing = 1,
	GS_Thrust = 2
};

UENUM(BlueprintType)
enum EGrappleThrustPhase
{
	GTP_Idle = 0,
	GTP_Throw = 1,
	GTP_Thrust = 2,
};

UENUM(BlueprintType)
enum EGrappleSwingPhase
{
	GSP_Idle = 0,
	GSP_Throw = 1,
	GSP_Swing = 3
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGrapplingStartEventSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGrapplingFinishEventSignature);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTGH_API UGrapplingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
	UClass* GrapplingHookClass;

	// The minimum and maximum distance between the character and the grapple point to grapple (swing and thrust)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
	FVector2D GrappleRange = FVector2D(700, 3500);;

	// The maximum angle between vector from character and grapple point and look vector in order to grapple
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
	float MaxAimAngleToGrapple = 16;

	// The collision channel used to determine obstruction between character and grapple points
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
	TEnumAsByte<ECollisionChannel> GrapplePointObstructionChannel = ECollisionChannel::ECC_WorldStatic;

	
	// The montage to play when throwing/starting the grapple on ground
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Animation Montages")
	UAnimMontage* GrappleThrowMontage = nullptr;

	// The montage to play when throwing/starting the grapple in air
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Animation Montages")
	UAnimMontage* GrappleThrowAirMontage = nullptr;

	// The montage to play when thrusting toward a grapple point above the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Animation Montages")
	UAnimMontage* GrappleThrustUpMontage = nullptr;

	// The montage to play when thrusting toward a grapple point below the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Animation Montages")
	UAnimMontage* GrappleThrustDownMontage = nullptr;

	// The montage to play upon releasing grapple swing and having upward velocity	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Animation Montages")
	UAnimMontage* SwingDismountMontage = nullptr;

	// The montage to play upon hitting the ground while swinging at a sideways angle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Animation Montages")
	UAnimMontage* KipUpMontage = nullptr;

	
	// Defines the bounds of the ground detector
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Swinging")
	FVector GroundDetectorVolumeExtent = FVector(25, 50, 100);

	// The maximum angle between character up vector and horizontal to kip up. Any more and character will not kip up
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Swinging")
	float MaxHorizAngleToKipUp = 55;

	// Determines the rate character rotates towards velocity while swinging
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Swinging")
	float SwingRotationRate = 8;
		
	// Whether should release grapple when feet hit the ground
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Swinging")
	bool bReleaseGrappleOnGrounded = true;

	// The minimum percent of the distance-needed-for-dismount-anim-to-finish-before-hitting-the-ground that will still do anim. ~confusing~
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling|Swinging")
	float MinPercentDistanceNeededToDismount = 0.5f;


	FGrapplingStartEventSignature GrapplingStartEventDelegate;
	FGrapplingFinishEventSignature GrapplingFinishEventDelegate;
	
	
	UGrapplingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void OnRegister() override;


	// === Public Common Grappling Methods ===
	void BindInput(UInputComponent* PlayerInputComponent);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SpinGrappleIcon(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent)
	void ResetGrappleIconAngle();
	
	
	UFUNCTION(BlueprintCallable)
	AGrapplePoint* GetBestValidGrapplePoint();

	UFUNCTION(BlueprintCallable)
	AGrapplingHook* GetGrapplingHook();

	UFUNCTION(BlueprintCallable)
	AGrapplePoint* GetCurrentGrapplePoint();
	
	TArray<AGrapplePoint*>* GetAvailableGrapplePoints();
	EGrappleState GetCurrentGrappleState();
	bool CanGrapple();

	
	// === Public Swinging Methods ===
	void SetGrappleSwingPhase(EGrappleSwingPhase _GrappleSwingPhase);
	void StartSwingPhase();
	void ReleaseGrappleFromSwing();
	
	UFUNCTION(BlueprintCallable)
	EGrappleSwingPhase GetGrappleSwingPhase();


	// === Public Thrusting Methods ===
	void StartGrappleThrust();
	void SetGrappleThrustPhase(EGrappleThrustPhase _GrappleThrustPhase);
	void FinishGrappleThrust();

	UFUNCTION(BlueprintCallable)
	EGrappleThrustPhase GetGrappleThrustPhase();

	
protected:
	virtual void BeginPlay() override;\

	
private:
	// === Common variables ===
	ACharacter* Character = nullptr;
	UCharacterMovementComponent* CharacterMovement = nullptr;

	USphereComponent* GrapplePointDetectionVolume = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;
	TArray<AGrapplePoint*> AvailableGrapplePoints;
	
	TEnumAsByte<EGrappleState> CurrentGrappleState = EGrappleState::GS_None;
	AGrapplePoint* BestValidGrapplePoint = nullptr;
	AGrapplePoint* CurrentGrapplePoint = nullptr;
	bool bCanGrapple = true;
	
	FVector LastFrameVelocity = FVector::ZeroVector;

	
	// === Swinging variables ===
	UBoxComponent* GroundDetectionVolume = nullptr;

	EGrappleSwingPhase GrappleSwingPhase = EGrappleSwingPhase::GSP_Idle;
	bool bHoldingSwingInput = false;
	float InitSwingDist = 0;
	bool bCanSwingWhileOnGround = false;

	
	// === Thrusting variables ===
	EGrappleThrustPhase GrappleThrustPhase = EGrappleThrustPhase::GTP_Idle;

	
	// === Private Common grappling methods ===
	void InitDetectionVolume();
	void GetOverlappedGrapplePoints();
	void CreateGrapplingHookActor();
	
	UFUNCTION()
	void OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void FindBestValidGrapplePoint();


	// === Private Swinging methods ===
	void TryGrappleSwing();
	void BeginSwingSequence();
	void SwingPhaseTick(float DeltaTime);
	void ReleaseGrappleSwingInput();
	


	UFUNCTION()
	void OnGroundOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void InitGroundDetectorVolume();
	void SetSwingEndActorRotation();
	bool CanDoAnimatedDismount();


	// === Private Thrusting Methods ===
	void TryGrappleThrust();
	void StartGrappleThrustSequence();
};
