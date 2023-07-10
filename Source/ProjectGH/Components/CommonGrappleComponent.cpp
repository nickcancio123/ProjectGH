// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/CommonGrappleComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Widgets/GrappleIconWidget.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"


#pragma region Default Actor Component Methods
UCommonGrappleComponent::UCommonGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	DetectionVolume = CreateDefaultSubobject<USphereComponent>("Detection Volume");
}

void UCommonGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());
	
	GetOverlappedGrapplePoints();
	CreateGrapplingHookActor();

	GrappleIconWidget =
		CreateWidget<UGrappleIconWidget>(GetWorld()->GetFirstPlayerController(), GrappleIconWidgetClass);
}

void UCommonGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FindBestValidGrapplePoint();
	UpdateGrappleIconWidget();
}

void UCommonGrappleComponent::OnRegister()
{
	Super::OnRegister();
	
	InitDetectionVolume();
}
#pragma endregion



#pragma region Grapple Driver Methods
void UCommonGrappleComponent::InitDetectionVolume()
{
	DetectionVolume->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	DetectionVolume->SetRelativeLocation(FVector::ZeroVector);
	
	DetectionVolume->SetSphereRadius(GrappleRange.Y);
	DetectionVolume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	DetectionVolume->SetCanEverAffectNavigation(false);
	
	DetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &UCommonGrappleComponent::OnOverlapStart);
	DetectionVolume->OnComponentEndOverlap.AddDynamic(this, &UCommonGrappleComponent::OnOverlapEnd);
}

void UCommonGrappleComponent::GetOverlappedGrapplePoints()
{
	TArray<AActor*> OverlappingActors;
	DetectionVolume->GetOverlappingActors(OverlappingActors, AGrapplePoint::StaticClass());

	for (int i = 0; i < OverlappingActors.Num(); i++)
		AvailableGrapplePoints.Add(Cast<AGrapplePoint>(OverlappingActors[i]));
}

void UCommonGrappleComponent::CreateGrapplingHookActor()
{
	GrapplingHook = Cast<AGrapplingHook>(GetWorld()->SpawnActor(GrapplingHookClass));
	if (!GrapplingHook)
		return;

	GrapplingHook->SetCommonGrappleComp(this);
	GrapplingHook->SetupGrapplingHook(Character->GetMesh());
	GrapplingHook->SetGrapplingHookState(EGrapplingHookState::GHS_In);
}


void UCommonGrappleComponent::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Add(GP);
}

void UCommonGrappleComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Remove(GP);
}


void UCommonGrappleComponent::FindBestValidGrapplePoint()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	Character->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector LineOfSight = ViewRotation.Vector().GetSafeNormal();

	float MinGPAngle = 361;
	AGrapplePoint* BestGrapplePointYet = nullptr;

	// Check each available GP if valid to grapple to, if so keep track of best option
	
	for (int i = 0; i < AvailableGrapplePoints.Num(); i++)
	{
		AGrapplePoint* GrapplePoint = AvailableGrapplePoints[i];
		
		// Get normalized vector from view location to GP
		FVector CamToGP =  GrapplePoint->GetActorLocation() - ViewLocation;
		float DistToGP = CamToGP.Size();
		CamToGP.Normalize();
		
		bool bValidDist = DistToGP > GrappleRange.X;
		if (!bValidDist)
			continue;

		// Check if angle is valid
		float Angle = FMath::Acos(FVector::DotProduct(LineOfSight, CamToGP));
		Angle = FMath::RadiansToDegrees(Angle);
		
		bool bValidAngle = Angle < MaxAimAngleToGrapple;
		if (!bValidAngle)
			continue;

		// Raycast to see if line of sight is obstructed to GP
		FHitResult HitResult;
		
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Character);
		CollisionParams.AddIgnoredActor(GrapplePoint);
		
		bool bTraceHit = GetWorld()->LineTraceSingleByProfile(
			HitResult,
			ViewLocation,
			GrapplePoint->GetActorLocation(),
			"BlockAll",
			CollisionParams
		);
		
		if (!bTraceHit)
		{
			if (Angle < MinGPAngle)
			{
				MinGPAngle = Angle;
				BestGrapplePointYet = GrapplePoint;	
			}
		}
	}
	
	BestValidGrapplePoint = BestGrapplePointYet ? BestGrapplePointYet : nullptr;
}
#pragma endregion



#pragma region Grapple Icon Widget Methods
void UCommonGrappleComponent::UpdateGrappleIconWidget()
{
	SetGrappleIconPosition();
	SetGrappleIconScale();
}

void UCommonGrappleComponent::SetGrappleIconPosition()
{
	bool bIconSpinning = GrappleIconWidget->GetRenderTransformAngle() != 0;
	GrappleIconGPRef = bIconSpinning ? GetCurrentGrapplePoint() : GetBestValidGrapplePoint();

	if (!GrappleIconGPRef)
	{
		if (GrappleIconWidget->IsInViewport())
			GrappleIconWidget->RemoveFromParent();
		return;
	}

	if (!GrappleIconWidget->IsInViewport())
		GrappleIconWidget->AddToViewport();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	FVector2D ScreenPos;
	PlayerController->ProjectWorldLocationToScreen(GrappleIconGPRef->GetActorLocation(), ScreenPos, true);

	ScreenPos += GrappleIconWidget->IconPositionOffset;
	GrappleIconWidget->SetPositionInViewport(ScreenPos);
}

void UCommonGrappleComponent::SetGrappleIconScale()
{
	if (!GrappleIconGPRef || !GrappleIconWidget->IsInViewport())
		return;

	FVector ViewPos;
	FRotator ViewRot;
	Character->GetActorEyesViewPoint(ViewPos, ViewRot);

	float GPViewDist = FVector::Dist(ViewPos, GrappleIconGPRef->GetActorLocation());

	float AlphaDist =
		FMath::GetMappedRangeValueClamped(GrappleRange, FVector2D(0, 1), GPViewDist);

	float ScaleAlpha = GrappleIconDistanceScaleCurve.GetRichCurve()->Eval(AlphaDist);
	float Scale = FMath::GetMappedRangeValueClamped(FVector2D(0, 1), GrappleIconScaleRange, ScaleAlpha);

	GrappleIconWidget->SetRenderScale(Scale * FVector2D(1, 1));
}

void UCommonGrappleComponent::SpinGrappleIcon(float DeltaTime)
{
	float Angle = GrappleIconWidget->GetRenderTransformAngle();
	Angle += DeltaTime * GrappleIconSpinRate;

	GrappleIconWidget->SetRenderTransformAngle(Angle);
}

void UCommonGrappleComponent::ResetGrappleIconAngle()
{
	GrappleIconWidget->SetRenderTransformAngle(0);
}
#pragma endregion 



#pragma region Setters
void UCommonGrappleComponent::SetCanGrapple(bool _bCanGrapple)
{	
	bCanGrapple = _bCanGrapple;
}

void UCommonGrappleComponent::SetCurrentGrapplePoint(AGrapplePoint* _CurrentGrapplePoint)
{
	CurrentGrapplePoint = _CurrentGrapplePoint;
}

void UCommonGrappleComponent::SetCurrentGrappleType(EGrappleType _GrappleType)
{
	CurrentGrappleType = _GrappleType;
}
#pragma endregion



#pragma region Getters
AGrapplePoint* UCommonGrappleComponent::GetBestValidGrapplePoint()
{
	return BestValidGrapplePoint;
}

AGrapplingHook* UCommonGrappleComponent::GetGrapplingHook()
{
	return GrapplingHook;
}

bool UCommonGrappleComponent::CanGrapple()
{
	return bCanGrapple;
}

TArray<AGrapplePoint*>* UCommonGrappleComponent::GetAvailableGrapplePoints()
{
	return &AvailableGrapplePoints;
}

AGrapplePoint* UCommonGrappleComponent::GetCurrentGrapplePoint()
{
	return CurrentGrapplePoint;
}

EGrappleType UCommonGrappleComponent::GetCurrentGrappleType()
{
	return CurrentGrappleType;
}
#pragma endregion
