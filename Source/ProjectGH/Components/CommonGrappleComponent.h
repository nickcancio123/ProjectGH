// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Actors/GrapplingHook.h"
#include "CommonGrappleComponent.generated.h"


class AGrapplePoint;
class AGrapplingHook;
class USphereComponent;



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTGH_API UCommonGrappleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Thrust")
		UClass* GrapplingHookClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Point Detector")
		FVector2D GrappleRange = FVector2D(700, 3000);;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Point Detector")
		float MaxAimAngleToGrapple = 20;
	
	UCommonGrappleComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnRegister() override;

	// Setters
	void SetCanGrapple(bool _bCanGrapple);
	void SetCurrentGrapplePoint(AGrapplePoint* _CurrentGrapplePoint);

	// Getters
	UFUNCTION(BlueprintCallable)
	AGrapplePoint* GetBestValidGrapplePoint();

	UFUNCTION(BlueprintCallable)
	AGrapplingHook* GetGrapplingHook();
	
	TArray<AGrapplePoint*>* GetAvailableGrapplePoints();
	AGrapplePoint* GetCurrentGrapplePoint();
	bool CanGrapple();

	
protected:
	virtual void BeginPlay() override;

	
private:
	ACharacter* Character = nullptr;
	USphereComponent* DetectionVolume = nullptr;
	AGrapplingHook* GrapplingHook = nullptr;
	TArray<AGrapplePoint*> AvailableGrapplePoints;

	AGrapplePoint* BestValidGrapplePoint = nullptr;
	AGrapplePoint* CurrentGrapplePoint = nullptr;
	
	bool bCanGrapple = true;
	
	
	void InitDetectionVolume();
	void GetOverlappedGrapplePoints();
	void CreateGrapplingHookActor();
	
	
	UFUNCTION()
	void OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void FindBestValidGrapplePoint();
};
