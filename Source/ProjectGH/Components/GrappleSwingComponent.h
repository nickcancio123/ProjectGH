// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GrappleSwingComponent.generated.h"


class AGrapplePoint;
class AGrapplingHook;
class UCommonGrappleComponent;

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
	UClass* GrapplingHookClass;
	
	
	UGrappleSwingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BindInput(UInputComponent* PlayerInputComponent);

	void ReleaseGrappleInput();
	
	
protected:
	virtual void BeginPlay() override;


private:
	ACharacter* Character = nullptr;
	UCharacterMovementComponent* CharacterMovement = nullptr;
	UCommonGrappleComponent* CommonGrappleComp = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;
	
	// Grapple driver methods
	void TryGrappleSwing();
};
