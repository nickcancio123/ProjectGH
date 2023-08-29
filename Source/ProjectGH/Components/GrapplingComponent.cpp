// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrapplingComponent.h"

#include "DrawDebugHelpers.h"
#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Actors/GrapplingHook.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"



#pragma region Actor Component Methods
UGrapplingComponent::UGrapplingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GrapplePointDetectionVolume = CreateDefaultSubobject<USphereComponent>("GrapplePoint Detection Volume");
	GroundDetectionVolume = CreateDefaultSubobject<UBoxComponent>("Ground Detection Volume");
}

void UGrapplingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());

	if (!Character)
		return;

	CharacterMovement = Character->GetCharacterMovement();
	
	GetOverlappedGrapplePoints();
	CreateGrapplingHookActor();
}

void UGrapplingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsActive())
		return;

	FindBestValidGrapplePoint();

	if (GrappleSwingPhase == EGrappleSwingPhase::GSP_Swing)
		SwingPhaseTick(DeltaTime);
}

void UGrapplingComponent::OnRegister()
{
	Super::OnRegister();

	InitDetectionVolume();
	InitGroundDetectorVolume();
}
#pragma endregion



#pragma region Common Grappling Methods
void UGrapplingComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	// Swinging 
	PlayerInputComponent->BindAction("GrappleSwing", IE_Pressed,this, &UGrapplingComponent::TryGrappleSwing);
	PlayerInputComponent->BindAction("GrappleSwing", IE_Released,this, &UGrapplingComponent::ReleaseGrappleSwingInput);

	// Thrusting
	PlayerInputComponent->BindAction("GrappleThrust", IE_Pressed,this, &UGrapplingComponent::TryGrappleThrust);
}

void UGrapplingComponent::InitDetectionVolume()
{
	GrapplePointDetectionVolume->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GrapplePointDetectionVolume->SetRelativeLocation(FVector::ZeroVector);
	
	GrapplePointDetectionVolume->SetSphereRadius(GrappleRange.Y);
	GrapplePointDetectionVolume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	GrapplePointDetectionVolume->SetCanEverAffectNavigation(false);
	
	GrapplePointDetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &UGrapplingComponent::OnOverlapStart);
	GrapplePointDetectionVolume->OnComponentEndOverlap.AddDynamic(this, &UGrapplingComponent::OnOverlapEnd);
}

void UGrapplingComponent::GetOverlappedGrapplePoints()
{
	TArray<AActor*> OverlappingActors;
	GrapplePointDetectionVolume->GetOverlappingActors(OverlappingActors, AGrapplePoint::StaticClass());

	for (int i = 0; i < OverlappingActors.Num(); i++)
		AvailableGrapplePoints.Add(Cast<AGrapplePoint>(OverlappingActors[i]));
}

void UGrapplingComponent::CreateGrapplingHookActor()
{
	GrapplingHook = Cast<AGrapplingHook>(GetWorld()->SpawnActor(GrapplingHookClass));
	if (!GrapplingHook)
		return;

	GrapplingHook->SetGrapplingCompRef(this);
	GrapplingHook->SetupGrapplingHook(Character->GetMesh());
	GrapplingHook->SetGrapplingHookState(EGrapplingHookState::GHS_In);
}


void UGrapplingComponent::OnOverlapStart(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Add(GP);
}

void UGrapplingComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;
	
	AGrapplePoint* GP = Cast<AGrapplePoint>(OtherActor);
	if (GP)
		AvailableGrapplePoints.Remove(GP);
}

void UGrapplingComponent::FindBestValidGrapplePoint()
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
		//CollisionParams.AddIgnoredActor(GrapplePoint);
		
		bool bTraceHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			ViewLocation,
			GrapplePoint->GetActorLocation(),
			GrapplePointObstructionChannel,
			CollisionParams
		);
		
		if (!bTraceHit || HitResult.GetActor() == GrapplePoint)
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


// === Getters ===
bool UGrapplingComponent::CanGrapple()
{
	return IsActive() && bCanGrapple && BestValidGrapplePoint;
}

AGrapplePoint* UGrapplingComponent::GetBestValidGrapplePoint()
{
	return BestValidGrapplePoint;
}

AGrapplingHook* UGrapplingComponent::GetGrapplingHook()
{
	return GrapplingHook;
}

TArray<AGrapplePoint*>* UGrapplingComponent::GetAvailableGrapplePoints()
{
	return &AvailableGrapplePoints;
}

AGrapplePoint* UGrapplingComponent::GetCurrentGrapplePoint()
{
	return CurrentGrapplePoint;
}

EGrappleState UGrapplingComponent::GetCurrentGrappleState()
{
	return CurrentGrappleState;
}
#pragma endregion 



#pragma region Swinging
void UGrapplingComponent::InitGroundDetectorVolume()
{
	GroundDetectionVolume->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GroundDetectionVolume->SetRelativeLocation(FVector(75, 0, 25));
	GroundDetectionVolume->SetBoxExtent(GroundDetectorVolumeExtent);
	
	GroundDetectionVolume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	GroundDetectionVolume->SetCanEverAffectNavigation(false);
	
	GroundDetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &UGrapplingComponent::OnGroundOverlap);
}

void UGrapplingComponent::TryGrappleSwing()
{
	bHoldingSwingInput = true;
	
	if (!CanGrapple())
		return;
	
	CurrentGrapplePoint = GetBestValidGrapplePoint();
	BeginSwingSequence();
}

void UGrapplingComponent::BeginSwingSequence()
{
	bCanGrapple = false;
	CurrentGrappleState = GS_Swing;
	GrappleSwingPhase = EGrappleSwingPhase::GSP_Throw;

	if (GrapplingStartEventDelegate.IsBound())
		GrapplingStartEventDelegate.Broadcast();
	
	UAnimMontage* ThrowMontage = CharacterMovement->IsFalling() ? GrappleThrowAirMontage : GrappleThrowMontage;
	Character->PlayAnimMontage(ThrowMontage);
}

void UGrapplingComponent::StartSwingPhase()
{
	if (!CurrentGrapplePoint)
		return;
	
	GrappleSwingPhase = EGrappleSwingPhase::GSP_Swing;

	// Set init swing dist
	FVector CharacterPos = Character->GetActorLocation();
	FVector GP_Pos = CurrentGrapplePoint->GetActorLocation();
	InitSwingDist = (GP_Pos - CharacterPos).Size();
	
	bCanSwingWhileOnGround = !CharacterMovement->IsFalling();

	LastFrameVelocity = CharacterMovement->Velocity;
	
	// Reset swing dismount montage length
	SwingDismountMontage->RateScale = 1;
}

void UGrapplingComponent::SwingPhaseTick(float DeltaTime)
{
	// End swing state condition
	if (!bHoldingSwingInput)
	{
		if (CharacterMovement->IsFalling() && CanDoAnimatedDismount())
		{
			GrappleSwingPhase = EGrappleSwingPhase::GSP_Idle;
			Character->PlayAnimMontage(SwingDismountMontage);
		}
		else
		{
			ReleaseGrappleFromSwing();
		}
		return;
	}

	
	FVector GP_Pos = CurrentGrapplePoint->GetActorLocation();
	FVector CharacterPos = Character->GetActorLocation();

	
	// Set new velocity
	FVector Vel = CharacterMovement->Velocity;
	FVector CharToGP = GP_Pos - CharacterPos;
	float Dist = CharToGP.Size();
	CharToGP.Normalize();
	
	FVector NewVel = Vel;
	if (Dist >= InitSwingDist)
	{
		FVector NewPos = GP_Pos + (-InitSwingDist * CharToGP);
		Character->SetActorLocation(NewPos);

		NewVel = FVector::VectorPlaneProject(Vel, CharToGP);
		
		// Add velocity to prevent velocity from zeroing at weird angles
		if (NewVel.Size() < 10)
			NewVel += 10 * NewVel.GetSafeNormal();
		
		CharacterMovement->Velocity = NewVel;
	}
	
	
	// Change rotation settings
	if (CharacterMovement->IsFalling())
	{
		bCanSwingWhileOnGround = false;
		CharacterMovement->bOrientRotationToMovement = false;
		
		FVector Velocity = CharacterMovement->Velocity;
		FVector Accel = (Velocity - LastFrameVelocity) / DeltaTime;
		
		float VelAccelDot = FVector::DotProduct(Velocity.GetSafeNormal(), Accel.GetSafeNormal());
		float VelAccelAngle = FMath::RadiansToDegrees(FMath::Acos(VelAccelDot));
		
		if (VelAccelAngle > 175 && Velocity.Size() < 800)
		{
			FVector CrossDir = FVector::CrossProduct(FVector::UpVector, CharToGP);
			CharacterMovement->AddImpulse(1000 * CrossDir);
		}

		FRotator CurrentRot = Character->GetActorRotation();
		FRotator TargetRot = UKismetMathLibrary::MakeRotFromXZ(CharacterMovement->Velocity, CharToGP);
		FRotator NewRot = UKismetMathLibrary::RInterpTo(CurrentRot, TargetRot, DeltaTime, SwingRotationRate);
		
		Character->SetActorRotation(NewRot);

		LastFrameVelocity = Velocity;
	}
	else
	{
		if (bReleaseGrappleOnGrounded && !bCanSwingWhileOnGround)
			ReleaseGrappleFromSwing();
		else
			SetSwingEndActorRotation();
	}
}	

void UGrapplingComponent::ReleaseGrappleSwingInput()
{
	bHoldingSwingInput = false;
}

void UGrapplingComponent::ReleaseGrappleFromSwing()
{
	bCanGrapple = true;
	CurrentGrappleState = EGrappleState::GS_None;
	GrappleSwingPhase = EGrappleSwingPhase::GSP_Idle;
	
	CharacterMovement->Velocity *= 1.25f;

	if (GrapplingFinishEventDelegate.IsBound())
		GrapplingFinishEventDelegate.Broadcast();
	
	GrapplingHook->SetGrapplingHookState(EGrapplingHookState::GHS_Pull);
	SetSwingEndActorRotation();
}

void UGrapplingComponent::OnGroundOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	if (!OtherActor)
		return;
	
	float Dot = FVector::DotProduct(Character->GetActorUpVector(), FVector::UpVector);
	float HorizAngle = 90 - FMath::RadiansToDegrees(FMath::Acos(Dot));

	FName Profile = OtherComp->GetCollisionProfileName();

	// Conditions to end grapple based on collision
	if (GrappleSwingPhase == EGrappleSwingPhase::GSP_Swing &&
		HorizAngle < MaxHorizAngleToKipUp &&
		(Profile.Compare("BlockAll") == 0 || Profile.Compare("BlockAllDynamic") == 0)
		)
	{
		Character->PlayAnimMontage(KipUpMontage);
		ReleaseGrappleFromSwing();
	}
}

bool UGrapplingComponent::CanDoAnimatedDismount()
{
	FVector Vel = CharacterMovement->Velocity;
	return Vel.Z > 0;
}


// === Getters & Setters===
EGrappleSwingPhase UGrapplingComponent::GetGrappleSwingPhase()
{
	return GrappleSwingPhase;
}

void UGrapplingComponent::SetGrappleSwingPhase(EGrappleSwingPhase _GrappleSwingPhase)
{
	GrappleSwingPhase = _GrappleSwingPhase;
}

void UGrapplingComponent::SetSwingEndActorRotation()
{
	FVector HorizVel = CharacterMovement->Velocity.GetSafeNormal2D();
	FVector NewLookDir = HorizVel.Size() > 250 ? HorizVel : Character->GetActorForwardVector().GetSafeNormal2D();
	
	Character->SetActorRotation(NewLookDir.Rotation());
	CharacterMovement->bOrientRotationToMovement = true;
}
#pragma endregion



#pragma region Thrusting
void UGrapplingComponent::TryGrappleThrust()
{
	if (!CanGrapple())
		return;

	CurrentGrapplePoint = BestValidGrapplePoint;
	StartGrappleThrustSequence();
}

void UGrapplingComponent::StartGrappleThrustSequence()
{
	bCanGrapple = false;
	CurrentGrappleState = EGrappleState::GS_Thrust;
	GrappleThrustPhase = EGrappleThrustPhase::GTP_Throw;

	if (GrapplingStartEventDelegate.IsBound())
		GrapplingStartEventDelegate.Broadcast();
	
	UAnimMontage* ThrowMontage = CharacterMovement->IsFalling() ? GrappleThrowAirMontage : GrappleThrowMontage;
	Character->PlayAnimMontage(ThrowMontage);
}

void UGrapplingComponent::StartGrappleThrust()
{	
	float GrapplePointZ = CurrentGrapplePoint->GetActorLocation().Z;

	CharacterMovement->bOrientRotationToMovement = false;
	
	UAnimMontage* MontageToPlay = GrapplePointZ >= Character->GetActorLocation().Z ? GrappleThrustUpMontage : GrappleThrustDownMontage;
	Character->PlayAnimMontage(MontageToPlay);
}

EGrappleThrustPhase UGrapplingComponent::GetGrappleThrustPhase()
{
	return GrappleThrustPhase;
}

void UGrapplingComponent::SetGrappleThrustPhase(EGrappleThrustPhase _GrappleThrustPhase)
{
	GrappleThrustPhase = _GrappleThrustPhase;
}

void UGrapplingComponent::FinishGrappleThrust()
{
	bCanGrapple = true;
	CurrentGrappleState = GS_None;
	GrappleThrustPhase = EGrappleThrustPhase::GTP_Idle;

	if (GrapplingFinishEventDelegate.IsBound())
		GrapplingFinishEventDelegate.Broadcast();
	
	// Set rotation 
	FVector HorizVel = CharacterMovement->Velocity;
	HorizVel.Z = 0;
	Character->SetActorRotation(HorizVel.Rotation());
	CharacterMovement->bOrientRotationToMovement = true;
}
#pragma endregion 

