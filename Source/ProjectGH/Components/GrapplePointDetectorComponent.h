// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectGH/Actors/GrapplePoint.h"
#include "GrapplePointDetectorComponent.generated.h"


class AGrapplePoint;
class USphereComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTGH_API UGrapplePointDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grapple Point Detector")
		float DetectionRadius = 3000;
	
	UGrapplePointDetectorComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnRegister() override;

	TArray<AGrapplePoint*>* GetAvailableGrapplePoints();
	
protected:
	virtual void BeginPlay() override;

private:
	USphereComponent* DetectionVolume = nullptr;
	TArray<AGrapplePoint*> AvailableGrapplePoints;
	
	void InitDetectionVolume();
	void GetOverlappedGrapplePoints();

	UFUNCTION()
	void OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
