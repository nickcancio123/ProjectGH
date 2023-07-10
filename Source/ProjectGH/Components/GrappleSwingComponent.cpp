// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGH/Components/GrappleSwingComponent.h"

#include "ProjectGH/Actors/GrapplePoint.h"
#include "ProjectGH/Components/CommonGrappleComponent.h"

#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"




#pragma region Default Actor Component Methods
UGrappleSwingComponent::UGrappleSwingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GroundDetectionVolume = CreateDefaultSubobject<UBoxComponent>("Ground Detection Volume");
}

void UGrappleSwingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetOwner())
		Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();

	CommonGrappleComp = Cast<UCommonGrappleComponent>(Character->GetComponentByClass(UCommonGrappleComponent::StaticClass()));
}

void UGrappleSwingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (GrappleSwingState == EGrappleSwingState::GSS_Swing)
		SwingStateTick(DeltaTime);
}

void UGrappleSwingComponent::OnRegister()
{
	Super::OnRegister();
	InitGroundDetectorVolume();
}
#pragma endregion



#pragma region Initializer Functions
void UGrappleSwingComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("GrappleSwing", IE_Pressed,this, &UGrappleSwingComponent::TryGrappleSwing);
	PlayerInputComponent->BindAction("GrappleSwing", IE_Released,this, &UGrappleSwingComponent::ReleaseGrappleInput);
}

void UGrappleSwingComponent::InitGroundDetectorVolume()
{
	GroundDetectionVolume->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GroundDetectionVolume->SetRelativeLocation(FVector(75, 0, 25));
	GroundDetectionVolume->SetBoxExtent(GroundDetectorVolumeExtent);
	
	GroundDetectionVolume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	GroundDetectionVolume->SetCanEverAffectNavigation(false);
	
	GroundDetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &UGrappleSwingComponent::OnGroundOverlap);
}
#pragma endregion 



#pragma region Grappling Driver Functions
void UGrappleSwingComponent::TryGrappleSwing()
{
	bHoldingInput = true;
	
	if (!CommonGrappleComp->CanGrapple())
		return;

	AGrapplePoint* BestValidGrapplePoint = CommonGrappleComp->GetBestValidGrapplePoint();
	if (BestValidGrapplePoint)
	{
		CommonGrappleComp->SetCurrentGrapplePoint(BestValidGrapplePoint);
		BeginSwingSequence();
		return;
	}

	CommonGrappleComp->SetCurrentGrapplePoint(nullptr);
}

void UGrappleSwingComponent::BeginSwingSequence()
{
	CommonGrappleComp->SetCanGrapple(false);
	CommonGrappleComp->SetCurrentGrappleType(EGrappleType::GT_Swing);
	
	GrappleSwingState = EGrappleSwingState::GSS_Throw;

	UAnimMontage* ThrowMontage = CharacterMovement->IsFalling() ? GrappleThrowAirMontage : GrappleThrowMontage;
	Character->PlayAnimMontage(ThrowMontage);
}

void UGrappleSwingComponent::StartSwingState()
{
	SetGrappleSwingState(EGrappleSwingState::GSS_Swing);

	// Set init swing dist
	FVector CharacterPos = Character->GetActorLocation();
	FVector GP_Pos = CommonGrappleComp->GetCurrentGrapplePoint()->GetActorLocation();
	InitSwingDist = (GP_Pos - CharacterPos).Size();
	
	bCanSwingWhileOnGround = !CharacterMovement->IsFalling();

	// Reset swing dismount montage length
	SwingDismountMontage->RateScale = 1;
}

void UGrappleSwingComponent::SwingStateTick(float DeltaTime)
{
	// End swing state condition
	if (!bHoldingInput)
	{
		if (CharacterMovement->IsFalling() && CanDoAnimatedDismount())
		{
			GrappleSwingState = EGrappleSwingState::GSS_Idle;
			Character->PlayAnimMontage(SwingDismountMontage);
		}
		else
		{
			ReleaseGrapple();
		}
		return;
	}

	
	FVector GP_Pos = CommonGrappleComp->GetCurrentGrapplePoint()->GetActorLocation();
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
		CharacterMovement->Velocity = NewVel;
	}

	
	// Change rotation settings
	if (CharacterMovement->IsFalling())
	{
		bCanSwingWhileOnGround = false;
		
		FRotator TargetRot = UKismetMathLibrary::MakeRotFromXZ(CharacterMovement->Velocity, CharToGP);
	
		FRotator NewRot = UKismetMathLibrary::RInterpTo(
			Character->GetActorRotation(),
			TargetRot,
			DeltaTime,
			SwingRotationRate
			);

		Character->SetActorRotation(NewRot);
	}
	else
	{
		if (bReleaseGrappleOnGrounded && !bCanSwingWhileOnGround)
			ReleaseGrapple();
		else
			SetSwingEndActorRotation();
	}
}

void UGrappleSwingComponent::ReleaseGrappleInput()
{
	bHoldingInput = false;
}

void UGrappleSwingComponent::ReleaseGrapple()
{
	CommonGrappleComp->SetCanGrapple(true);
	CommonGrappleComp->SetCurrentGrappleType(EGrappleType::GT_None);
	CommonGrappleComp->GetGrapplingHook()->SetGrapplingHookState(EGrapplingHookState::GHS_Pull);
	
	GrappleSwingState = EGrappleSwingState::GSS_Idle;
	  	
	// Set rotation 
	SetSwingEndActorRotation();
}

void UGrappleSwingComponent::OnGroundOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	if (!OtherActor)
		return;
	
	float Dot = FVector::DotProduct(Character->GetActorUpVector(), FVector::UpVector);
	float HorizAngle = 90 - FMath::RadiansToDegrees(FMath::Acos(Dot));

	FName Profile = OtherComp->GetCollisionProfileName();

	// Conditions to end grapple based on collision
	if (GrappleSwingState == EGrappleSwingState::GSS_Swing &&
		HorizAngle < MaxHorizAngleToKipUp &&
		(Profile.Compare("BlockAll") == 0 || Profile.Compare("BlockAllDynamic") == 0)
		)
	{
		Character->PlayAnimMontage(KipUpMontage);
		ReleaseGrapple();
	}
}

bool UGrappleSwingComponent::CanDoAnimatedDismount()
{
	FVector Pos = Character->GetActorLocation();
	FVector Vel = CharacterMovement->Velocity;
	
	float AnimLen = SwingDismountMontage->SequenceLength;
	float Grav = CharacterMovement->GravityScale * 9.81f;
	float AccelAmount = 0.5f * FMath::Pow(Grav, 2);

	if (Vel.Z > AccelAmount)
		return true;

	FVector TargetPos = Pos + (AnimLen * Vel) + (AccelAmount * FVector::DownVector);
	float TargetDist = FVector::Dist(Pos, TargetPos);
	
	FHitResult HitInfo;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, Pos, TargetPos, ECC_Visibility);

	if (bHit)
	{
		float PercentDist = HitInfo.Distance / TargetDist;
		if (PercentDist < MinPercentDistanceNeededToDismount)
			return false;
		
		SwingDismountMontage->RateScale = 1 / PercentDist;
		return true;
	}
	
	return true;
}
#pragma endregion 



#pragma region Getters
EGrappleSwingState UGrappleSwingComponent::GetGrappleSwingState()
{
	return GrappleSwingState;
}

#pragma endregion



#pragma region Setters
void UGrappleSwingComponent::SetGrappleSwingState(EGrappleSwingState _GrappleSwingState)
{
	GrappleSwingState = _GrappleSwingState;
}

void UGrappleSwingComponent::SetSwingEndActorRotation()
{
	FVector HorizVel = CharacterMovement->Velocity.GetSafeNormal2D();
	FVector NewLookDir = HorizVel.Size() > 250 ? HorizVel : Character->GetActorForwardVector().GetSafeNormal2D();
	
	Character->SetActorRotation(NewLookDir.Rotation());
	CharacterMovement->bOrientRotationToMovement = true;
}
#pragma endregion
