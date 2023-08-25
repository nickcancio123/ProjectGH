// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapplingComponent.generated.h"


UENUM()
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


	
	UGrapplingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:
	virtual void BeginPlay() override;
};
