// Fill out your copyright notice in the Description page of Project Settings.


#include "ObstacleTraversalComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"


#pragma region Actor Component Methods
UObstacleTraversalComponent::UObstacleTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UObstacleTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Null character reference"));
		return;
	}
	CharacterMovementComp = Cast<UCharacterMovementComponent>(Character->GetComponentByClass(UCharacterMovementComponent::StaticClass()));

	CapsuleComp = Cast<UCapsuleComponent>(Character->GetComponentByClass(UCapsuleComponent::StaticClass()));

	
	TraceCollisionQueryParams.AddIgnoredActor(Character);

	// Create collision shape
	CollisionShape.ShapeType = ECollisionShape::Sphere;
	CollisionShape.SetSphere(TraceSphereRadius);

	ComputeMaxObstacleHeight();
}
#pragma endregion 



#pragma region Public Methods
void UObstacleTraversalComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("TraverseObstacle", IE_Pressed, this, &UObstacleTraversalComponent::TryTraverse);

	PlayerInputComponent->BindAxis("MoveForward", this, &UObstacleTraversalComponent::OnForwardInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &UObstacleTraversalComponent::OnRightInput);
}

void UObstacleTraversalComponent::AllowTraversal()
{
	bCanTraverse = true;

	if (CapsuleComp)
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (Mesh)
	{
		if (Mesh->GetAnimInstance())
		{
			Mesh->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		}
	}
	
	if (CharacterMovementComp)
	{
		CharacterMovementComp->SetMovementMode(EMovementMode::MOVE_Walking);

		WorldMoveInput = GetWorldMoveInput();
		if (WorldMoveInput.Size() > 0)
		{
			CharacterMovementComp->Velocity = CharacterMovementComp->MaxWalkSpeed * WorldMoveInput;
		}
	}
}
#pragma endregion



#pragma region Obstacle Traversal Driver Methods

void UObstacleTraversalComponent::TryTraverse()
{
	if (!bCanTraverse)
		return;
	
	if (!Character)
		return;

	World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Null world reference"));
		return;
	}
	
	CurrentCharFeetPos = Character->GetActorLocation() + CharacterFeetHeightOffset * FVector::UpVector;

	WorldMoveInput = GetWorldMoveInput();


	// Find obstacle
	bool bObjectFound = FindObstacleTowardInputDirection();
	
	if (!bObjectFound)
		return;

	// Get obstacle info
	bool bFoundAllInfo = GetObstacleInfo();

	if (!bFoundAllInfo)
		return;


	// Try traversal
	if (bTryVaultFirst)
	{
		if (!TryVault())
			TryClimb();
	}
	else
	{
		if (!TryClimb())
			TryVault();
	}
}



bool UObstacleTraversalComponent::FindObstacleTowardInputDirection()
{
	if (ObstacleQueryCastHeights.Num() < 1)
		return false;
	
	for (int i = 0; i < ObstacleQueryCastHeights.Num(); i++)
	{
		float TraceHeight = ObstacleQueryCastHeights[i];
		FVector TraceStart = CurrentCharFeetPos + TraceHeight * FVector::UpVector;
		FVector TraceEnd = TraceStart + ObstacleQueryRange * WorldMoveInput;

		FHitResult HitInfo;

		bool bHit =
			World->SweepSingleByChannel(HitInfo, TraceStart, TraceEnd, FQuat::Identity, ObstacleCollisionChannel, CollisionShape, TraceCollisionQueryParams);

		if (bShowDebug)
		{
			DrawDebugLine(World, TraceStart, bHit ? HitInfo.ImpactPoint : TraceEnd, FColor::Blue, false, DebugSymbolsLifeTime, 0, 2);

			if (bHit)
				DrawDebugSphere(World, HitInfo.Location, TraceSphereRadius, 16, FColor::Blue, false, DebugSymbolsLifeTime);
		}

		if (bHit)
		{
			// The angle of the impact normal from the up vector, representing the incline angle. A nearly flat surface/ramp would be near 0. A sheer wall would be nearly 90.
			float HorizontalAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::UpVector, HitInfo.ImpactNormal)));
			if (HorizontalAngle < CharacterMovementComp->GetWalkableFloorAngle())
				continue;
			
			CurrentObstacleInfo.ObstacleQueryInfo = HitInfo;
			return true;
		}
	}

	return false;
}

bool UObstacleTraversalComponent::GetObstacleInfo()
{
	// Get obstacle face distance to character
	CurrentObstacleInfo.DistanceFromCharacter = CurrentObstacleInfo.ObstacleQueryInfo.Distance + TraceSphereRadius;
	
	// Get obstacle direction
	bool bValidObstacleDirection = GetObstacleDepthDirection();
	
	if (!bValidObstacleDirection)
		return false;
	
	// Get obstacle height
	bool bFoundObstacleHeight = GetObstacleHeight();
	
	if (!bFoundObstacleHeight)
		return false;
	
	// Get obstacle depth
	bool bFoundObstacleDepth = GetObstacleDepth();
	
	if (!bFoundObstacleDepth)
		return false;

	// Get obstacle front and back ledge
	bool bFoundLedges = ComputeObstacleLedges();
	
	if (!bFoundLedges)
		return false;

	
	return true;
}



bool UObstacleTraversalComponent::GetObstacleDepthDirection()
{
	CurrentObstacleInfo.FaceNormal = CurrentObstacleInfo.ObstacleQueryInfo.ImpactNormal;

	// Compute angle between negative obstacle face and world move input
	float AngleRadians = FMath::Acos(FVector::DotProduct(-CurrentObstacleInfo.ObstacleQueryInfo.ImpactNormal, WorldMoveInput));
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	if (AngleDegrees > MaxAngleToObstacleFaceNormal)
		return false;

	CurrentObstacleInfo.DepthDirection = WorldMoveInput;

	if (bShowDebug)
	{
		FVector Start = CurrentObstacleInfo.ObstacleQueryInfo.ImpactPoint + 100 * FVector::UpVector;
		FVector End = Start + 100 * CurrentObstacleInfo.DepthDirection;
		DrawDebugLine(World, Start, End, FColor::Purple, false, DebugSymbolsLifeTime, 0, 2);
	}

	return true;
}

bool UObstacleTraversalComponent::GetObstacleHeight()
{
	float HeightTraceStartHeight = 5 + MaxObstacleHeight + CurrentCharFeetPos.Z;
	
	FVector HeightTraceEnd = CurrentObstacleInfo.ObstacleQueryInfo.ImpactPoint + 20 * CurrentObstacleInfo.DepthDirection;
	FVector HeightTraceStart = FVector(HeightTraceEnd.X, HeightTraceEnd.Y, HeightTraceStartHeight);
	
	bool bHit =
		World->SweepSingleByChannel(CurrentObstacleInfo.HeightTraceInfo, HeightTraceStart, HeightTraceEnd, FQuat::Identity, ObstacleCollisionChannel, CollisionShape, TraceCollisionQueryParams);
	
	if (bShowDebug)
	{
		DrawDebugLine(World, HeightTraceStart, HeightTraceEnd, FColor::Yellow, false, DebugSymbolsLifeTime, 0, 2);
		if (bHit)
			DrawDebugSphere(World, CurrentObstacleInfo.HeightTraceInfo.Location, TraceSphereRadius, 16, FColor::Yellow, false, DebugSymbolsLifeTime);
	}

	if (!bHit)
		return false;
	
	CurrentObstacleInfo.Height = CurrentObstacleInfo.HeightTraceInfo.ImpactPoint.Z - CurrentCharFeetPos.Z;
	//UE_LOG(LogTemp, Warning, TEXT("Obstacle Height: %f"), CurrentObstacleInfo.Height);	
	
	return true;
}

bool UObstacleTraversalComponent::GetObstacleDepth()
{
	FVector DepthTraceStart = CurrentObstacleInfo.ObstacleQueryInfo.ImpactPoint + (MaxObstacleDepthToVault + 20) * CurrentObstacleInfo.DepthDirection;
	FVector DepthTraceEnd = CurrentObstacleInfo.ObstacleQueryInfo.ImpactPoint;
	
	bool bHit =
		World->SweepSingleByChannel(CurrentObstacleInfo.DepthTraceInfo, DepthTraceStart, DepthTraceEnd, FQuat::Identity, ObstacleCollisionChannel, CollisionShape, TraceCollisionQueryParams);

	
	if (bShowDebug)
	{
		DrawDebugLine(World, DepthTraceStart, bHit ? CurrentObstacleInfo.DepthTraceInfo.ImpactPoint : DepthTraceEnd, FColor::Cyan, false, DebugSymbolsLifeTime, 0, 3);
		if (bHit)
			DrawDebugSphere(World, CurrentObstacleInfo.DepthTraceInfo.Location, TraceSphereRadius, 16, FColor::Cyan, false, DebugSymbolsLifeTime);
	}
		
	if (!bHit)
		return false;	
		
	float Depth = CurrentObstacleInfo.DepthTraceInfo.bStartPenetrating ? MaxObstacleDepthToVault + 5 : FVector::Dist(CurrentObstacleInfo.DepthTraceInfo.ImpactPoint + TraceSphereRadius, DepthTraceEnd);
	
	CurrentObstacleInfo.Depth = Depth;
	
	//UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth: %f"), CurrentObstacleInfo.Depth);

	return true;
}

bool UObstacleTraversalComponent::ComputeObstacleLedges()
{
	// Compute front ledge
	FVector TmpFrontLedge = CurrentObstacleInfo.ObstacleQueryInfo.ImpactPoint;
	TmpFrontLedge.Z = CurrentObstacleInfo.HeightTraceInfo.ImpactPoint.Z;
	CurrentObstacleInfo.FrontLedgePos = TmpFrontLedge;

	
	// Compute back ledge
	CurrentObstacleInfo.BackLedgePos = CurrentObstacleInfo.FrontLedgePos + CurrentObstacleInfo.Depth * CurrentObstacleInfo.DepthDirection;

	if (bShowDebug)
	{
		DrawDebugSphere(World, CurrentObstacleInfo.FrontLedgePos, 5, 16, FColor::Magenta, false, DebugSymbolsLifeTime);
		DrawDebugSphere(World, CurrentObstacleInfo.BackLedgePos, 5, 16, FColor::Magenta, false, DebugSymbolsLifeTime);
	}

	return true;
}



bool UObstacleTraversalComponent::TryVault()
{
	// Can't vault while in air/falling
	if (CharacterMovementComp->IsFalling())
		return false;


	// Check obstacle depth
	if (CurrentObstacleInfo.Depth > MaxObstacleDepthToVault)
		return false;

	
	// Compute start pos
	CurrentVaultInfo.StartPos = Character->GetActorLocation();


	// Compute land pos
	FHitResult LandTraceInfo;
	FVector LandTraceEnd = CurrentObstacleInfo.ObstacleQueryInfo.ImpactPoint + (CurrentObstacleInfo.Depth + VaultLandDistance) * CurrentObstacleInfo.DepthDirection;
	LandTraceEnd.Z = CurrentCharFeetPos.Z + VaultLandHeightRange.X;
	
	float LandTraceStartHeight = 5 + MaxObstacleHeight + VaultLandHeightRange.Y + CurrentCharFeetPos.Z;
	FVector LandTraceStart = FVector(LandTraceEnd.X, LandTraceEnd.Y, LandTraceStartHeight);
	
	bool bHitLand =
		World->SweepSingleByChannel(LandTraceInfo, LandTraceStart, LandTraceEnd, FQuat::Identity, ObstacleCollisionChannel, CollisionShape, TraceCollisionQueryParams);


	if (!bHitLand)
		return false;
	
	CurrentVaultInfo.LandPos = LandTraceInfo.ImpactPoint;


	// Select vault variation
	if (VaultVariants.Num() < 1)
		return false;
	for (int v = 0; v < VaultVariants.Num(); v++)
	{
		FVaultVariant Variant = VaultVariants[v];

		if (CurrentObstacleInfo.Height < Variant.MaxObstacleHeight)
		{
			CurrentVaultInfo.CurrentVaultVariant = Variant;
			StartVault();
			return true;
		}
	}

	// Could not find valid variation
	return false;
}

bool UObstacleTraversalComponent::TryClimb()
{
	if (!bCanClimbInAir && CharacterMovementComp->IsFalling())
		return false;

	// Compute start pos
	CurrentClimbInfo.StartPos = Character->GetActorLocation();

	// Select climb variation
	if (ClimbVariants.Num() < 1)
		return false;
	
	for (int v = 0; v < ClimbVariants.Num(); v++)
	{
		FClimbVariant Variant = ClimbVariants[v];
		
		if (CurrentObstacleInfo.Height < Variant.MaxObstacleHeight)
		{
			// If not valid distance, can't climb altogether
			float MaxDistanceToCharacter = Variant.bUseCustomMaxDistanceToObstacle ? Variant.CustomMaxDistanceToObstacle : ObstacleQueryRange;
			bool bValidDistanceToCharacter = CurrentObstacleInfo.DistanceFromCharacter < MaxDistanceToCharacter;
		
			if (!bValidDistanceToCharacter)
				break;
			
			CurrentClimbInfo.CurrentClimbVariant = Variant;
			StartClimb();
			
			return true;
		}
	}

	// Could not find valid variation
	return false;
}


void UObstacleTraversalComponent::StartVault()
{
	// Compute recover position if necessary
	if (CurrentVaultInfo.CurrentVaultVariant.bHasRecoverState)
	{
		FVector RecoverTraceEnd = CurrentVaultInfo.LandPos +
			CurrentVaultInfo.CurrentVaultVariant.RecoverDistance * CurrentObstacleInfo.DepthDirection +
				20 * FVector::DownVector;
		FVector RecoverTraceStart = RecoverTraceEnd + 100 * FVector::UpVector;

		FHitResult RecoverTraceInfo;
		
		bool bHit =
			World->SweepSingleByChannel(RecoverTraceInfo, RecoverTraceStart, RecoverTraceEnd, FQuat::Identity, ObstacleCollisionChannel, CollisionShape, TraceCollisionQueryParams);
		
		if (!bHit)
			return;
	
		CurrentVaultInfo.RecoverPos = RecoverTraceInfo.ImpactPoint;
	}

	
	if (!CurrentVaultInfo.CurrentVaultVariant.AnimMontage)
		return;

	
	StartTraversal(ETraversalType::TT_VAULT);

	
	if (bShowDebug)
	{
		DrawDebugSphere(World, CurrentVaultInfo.StartPos, 5, 16, FColor::Orange, false, DebugSymbolsLifeTime);
		DrawDebugSphere(World, CurrentVaultInfo.LandPos, 5, 16, FColor::Orange, false, DebugSymbolsLifeTime);

		if (CurrentVaultInfo.CurrentVaultVariant.bHasRecoverState)
			DrawDebugSphere(World, CurrentVaultInfo.RecoverPos, 5, 16, FColor::Orange, false, DebugSymbolsLifeTime);
	}
}

void UObstacleTraversalComponent::StartClimb()
{
	if (!CurrentClimbInfo.CurrentClimbVariant.AnimMontage)
		return;

	StartTraversal(ETraversalType::TT_CLIMB);
	
	if (bShowDebug)
	{
		DrawDebugSphere(World, CurrentClimbInfo.StartPos, 5, 16, FColor::Orange, false, DebugSymbolsLifeTime);
	}
}

void UObstacleTraversalComponent::StartTraversal(ETraversalType TraversalType)
{
	CurrentTraversalType = TraversalType;

	// Play montage
	if (TraversalType == ETraversalType::TT_VAULT)
		Character->PlayAnimMontage(CurrentVaultInfo.CurrentVaultVariant.AnimMontage);
	else if (TraversalType == ETraversalType::TT_CLIMB)
		Character->PlayAnimMontage(CurrentClimbInfo.CurrentClimbVariant.AnimMontage);
	
	bCanTraverse = false;
	
	if (ObstacleTraversalStartEventDelegate.IsBound())
		ObstacleTraversalStartEventDelegate.Broadcast();
}



void UObstacleTraversalComponent::OnVaultComplete()
{
	CompleteTraversal(TT_VAULT);
}

void UObstacleTraversalComponent::OnClimbComplete()
{
	// Disable root motion to enable post climb speed
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (Mesh && Mesh->GetAnimInstance())
	{
		Mesh->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}
	
	CompleteTraversal(ETraversalType::TT_CLIMB);
}

void UObstacleTraversalComponent::CompleteTraversal(ETraversalType TraversalType)
{
	bCanTraverse = true;
	
	CurrentTraversalType = TT_NONE;

	if (ObstacleTraversalCompleteEventDelegate.IsBound())
		ObstacleTraversalCompleteEventDelegate.Broadcast();

	// Enable collision
	if (CapsuleComp)
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	if (!CharacterMovementComp)
		return;

	// Set move mode
	CharacterMovementComp->SetMovementMode(EMovementMode::MOVE_Walking);

	// Set post vault speed
	WorldMoveInput = GetWorldMoveInput();
	if (WorldMoveInput.Size() > 0)
	{
		float PostTraversalSpeed = 0;
		if (TraversalType == TT_VAULT)
			PostTraversalSpeed = CurrentVaultInfo.CurrentVaultVariant.bUseCustomPostVaultSpeed ? CurrentVaultInfo.CurrentVaultVariant.CustomPostVaultSpeed : CharacterMovementComp->MaxWalkSpeed;
		else
			PostTraversalSpeed = CurrentClimbInfo.CurrentClimbVariant.bUseCustomPostClimbSpeed ? CurrentClimbInfo.CurrentClimbVariant.CustomPostClimbSpeed : CharacterMovementComp->MaxWalkSpeed;

		CharacterMovementComp->Velocity = PostTraversalSpeed * WorldMoveInput;
	}
}
#pragma endregion



#pragma region Getter Methods
ETraversalType UObstacleTraversalComponent::GetCurrentTraversalType()
{
	return CurrentTraversalType;
}

FVector UObstacleTraversalComponent::GetTargetVaultPosition(EVaultState VaultState)
{
	FVector Forward = -CurrentObstacleInfo.FaceNormal;
	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(Forward, Up);

	FVector TargetPositionOffset;
	FVector PositionReference;
	if (VaultState == EVaultState::VS_MOUNT)
	{
		TargetPositionOffset = CurrentVaultInfo.CurrentVaultVariant.Mount_TargetPosOffset;
		PositionReference = CurrentObstacleInfo.FrontLedgePos;
	}
	else if (VaultState == EVaultState::VS_OVER)
	{
		TargetPositionOffset = CurrentVaultInfo.CurrentVaultVariant.Over_TargetPosOffset;
		PositionReference = CurrentObstacleInfo.BackLedgePos;
	}
	else if (VaultState == EVaultState::VS_DISMOUNT)
	{
		TargetPositionOffset = CurrentVaultInfo.CurrentVaultVariant.Dismount_TargetPosOffset;
		PositionReference = CurrentVaultInfo.LandPos;
	}
	else if (VaultState == EVaultState::VS_RECOVER)
	{
		TargetPositionOffset = CurrentVaultInfo.CurrentVaultVariant.Recover_TargetPosOffset;
		PositionReference = CurrentVaultInfo.RecoverPos;
	}

	FVector TargetPos = PositionReference + (TargetPositionOffset.X * Forward) + (TargetPositionOffset.Y * Right) + (TargetPositionOffset.Z * Up);

	if (bShowDebug)
		DrawDebugSphere(GetWorld(), TargetPos, 10, 10, FColor::Magenta, false, DebugSymbolsLifeTime);

	return TargetPos;
}

FRotator UObstacleTraversalComponent::GetTargetVaultRotation()
{
	//FVector Forward = -CurrentObstacleInfo.FaceNormal;
	FVector Forward = CurrentObstacleInfo.DepthDirection;
	Forward.Z = 0;
	Forward.Normalize();
	
	FRotator TargetRot = UKismetMathLibrary::MakeRotFromXZ(Forward, FVector::UpVector);
	return TargetRot;
}


FVector UObstacleTraversalComponent::GetTargetClimbMountPosition()
{
	FVector Forward = -CurrentObstacleInfo.FaceNormal;
	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(Forward, Up);

	FVector MountOffset = CurrentClimbInfo.CurrentClimbVariant.Mount_TargetPosOffset;
	FVector TargetPos = CurrentObstacleInfo.FrontLedgePos + (MountOffset.X * Forward) + (MountOffset.Y * Right) + (MountOffset.Z * Up);

	if (bShowDebug)
		DrawDebugSphere(GetWorld(), TargetPos, 10, 10, FColor::Magenta, false, DebugSymbolsLifeTime);

	return TargetPos;
}

FRotator UObstacleTraversalComponent::GetTargetClimbRotation()
{
	FVector Forward = CurrentClimbInfo.CurrentClimbVariant.bUseDepthDirectionForRotation ? CurrentObstacleInfo.DepthDirection : -CurrentObstacleInfo.FaceNormal;
	Forward.Z = 0;
	Forward.Normalize();
	
	FRotator TargetRot = UKismetMathLibrary::MakeRotFromXZ(Forward, FVector::UpVector);
	return TargetRot;
}
#pragma endregion



#pragma region Misc
void UObstacleTraversalComponent::ComputeMaxObstacleHeight()
{
	if (VaultVariants.Num() > 0)
	{
		for (int v = 0; v < VaultVariants.Num(); v++)
		{
			if (VaultVariants[v].MaxObstacleHeight > MaxObstacleHeight)
				MaxObstacleHeight = VaultVariants[v].MaxObstacleHeight;
		}
	}

	if (ClimbVariants.Num() < 0)
		return;
	
	for (int c = 0; c < ClimbVariants.Num(); c++)
	{
		if (ClimbVariants[c].MaxObstacleHeight > MaxObstacleHeight)
			MaxObstacleHeight = ClimbVariants[c].MaxObstacleHeight;
	}
}


void UObstacleTraversalComponent::OnForwardInput(float InputValue)
{
	MoveInput.X = InputValue;
}

void UObstacleTraversalComponent::OnRightInput(float InputValue)
{
	MoveInput.Y = InputValue;
}

FVector UObstacleTraversalComponent::GetWorldMoveInput()
{
	// Get world move input
	FVector CharControllerForward = FVector::VectorPlaneProject(Character->GetControlRotation().Vector(), FVector::UpVector);
	FVector CharControllerRight = FVector::CrossProduct(FVector::UpVector, Character->GetControlRotation().Vector());
	FVector TempWorldMoveInput = (MoveInput.X * CharControllerForward) + (MoveInput.Y * CharControllerRight);
	return TempWorldMoveInput.GetSafeNormal();
}
#pragma endregion

