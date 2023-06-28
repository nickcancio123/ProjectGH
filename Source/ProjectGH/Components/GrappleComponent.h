// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleComponent.generated.h"


class AGrapplePoint;
class AGrapplingHook;
class USphereComponent;
class UInputComponent;
class UAnimMontage;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTGH_API UGrappleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
	 	UClass* GrapplingHookClass;
	
	// Sphere collider used to detect grapple points
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
		USphereComponent* GP_Detector = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
		UAnimMontage* GrappleAnimMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
		FFloatRange GrappleRange = FFloatRange(700, 3000);
	
	// The max angle (degrees) between line-of-sight and vector to GP to consider for grappling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling")
		float Max_GP_SightAngle = 20;
	
	virtual void OnRegister() override;
	
	UGrappleComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// Setters
	void BindInput(UInputComponent* PlayerInputComponent);
	void SetCanGrapple(bool _bCanGrapple);

	// Getters
	UFUNCTION(BlueprintCallable)
	AGrapplePoint* GetBestValidGrapplePoint();
	
	AGrapplingHook* GetGrapplingHook();
	
	AGrapplePoint* GetCurrentGrapplePoint();
	
	FVector GetGrappleDirection();


	
protected:
	virtual void BeginPlay() override;

	

private:
	ACharacter* Character = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;

	// Set of GPs that are within GP detection range
	TArray<AGrapplePoint*> Available_GPs;

	// This frame's best valid GP option
	AGrapplePoint* BestValid_GP = nullptr;

	// GP currently begin grappled to 
	AGrapplePoint* Current_GP = nullptr;

	bool bCanGrapple = true;
	
	
	// Initializers
	void CreateGrappleHookActor();
	void InitGrapplePointDetector();
	void GetOverlapped_GPs();

	
	// Grapple driver functions
	UFUNCTION()
	void OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void TryGrapple();
	void FindBestValidGP();
	void BeginGrapple();
};
