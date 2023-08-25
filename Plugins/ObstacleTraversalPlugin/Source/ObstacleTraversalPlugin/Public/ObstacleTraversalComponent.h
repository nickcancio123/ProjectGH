// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "ObstacleTraversalComponent.generated.h"

	
UENUM()
enum ETraversalType
{
	TT_NONE = 0,
	TT_VAULT = 1,
	TT_CLIMB = 2
};

UENUM()
enum EVaultState
{
	VS_MOUNT = 0,		// Go to obstacle front ledge
	VS_OVER = 1,		// Go over obstacle to back ledge
	VS_DISMOUNT = 2,	// Go to ground (land)
	VS_RECOVER = 3		// Recover from vault (e.g., roll)
};


USTRUCT(BlueprintType)
struct FObstacleInfo
{
	GENERATED_BODY()

	// The results of query trace to find obstacle
	FHitResult ObstacleQueryInfo;

	// The results of trace to find height of obstacle
	FHitResult HeightTraceInfo;

	// The results of trace to find depth of obstacle
	FHitResult DepthTraceInfo;


	// The distance between the character and the face of the obstacle.
	float DistanceFromCharacter = 0;
	
	// The functional height of the obstacle. Determines which variation to use, if any
	float Height = 0;

	// The functional depth of the obstacle. Determines whether to vault, climb, or neither
	float Depth = 0;

	// The vector normal to the obstacle face hit by query trace
	FVector FaceNormal = FVector::ZeroVector;
	
	// The ledge to which character mounts
	FVector FrontLedgePos = FVector::ZeroVector;
	
	// The ledge at which character clears obstacle (only used for vaulting)
	FVector BackLedgePos = FVector::ZeroVector;

	// Whether the back ledge was successfully computed
	bool bBackLedgeFound = true;

	// The direction in which the obstacle will be traversed
	FVector DepthDirection = FVector::ZeroVector;
};


USTRUCT(BlueprintType)
struct FVaultVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(EditAnywhere)
	float MaxObstacleHeight = 0;
	
	UPROPERTY(EditAnywhere)
	bool bHasRecoverState = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition="bHasRecoverState"))
	float RecoverDistance = 100;


	// If set false, post vault speed will be character movement max walk speed
	UPROPERTY(EditAnywhere)
	bool bUseCustomPostVaultSpeed = false;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bUseCustomPostVaultSpeed"))
	float CustomPostVaultSpeed = 200;

	
	// Position offsets
	UPROPERTY(EditAnywhere)
	FVector Mount_TargetPosOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector Over_TargetPosOffset = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere)
	FVector Dismount_TargetPosOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, meta = (EditCondition="bHasRecoverState"))
	FVector Recover_TargetPosOffset = FVector::ZeroVector;
};


USTRUCT(BlueprintType)
struct FClimbVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(EditAnywhere)
	float MaxObstacleHeight = 0;

	// If false, uses negative obstacle face normal for rotation
	UPROPERTY(EditAnywhere)
	bool bUseDepthDirectionForRotation = false;

	
	UPROPERTY(EditAnywhere)
	bool bUseCustomMaxDistanceToObstacle = false;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bUseCustomMaxDistanceToObstacle"))
    float CustomMaxDistanceToObstacle = 100;


	// If set false, post climb speed will be character movement max walk speed
	UPROPERTY(EditAnywhere)
	bool bUseCustomPostClimbSpeed = false;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bUseCustomPostClimbSpeed"))
	float CustomPostClimbSpeed = 200;

	
	// Position offsets
	UPROPERTY(EditAnywhere)
	FVector Mount_TargetPosOffset = FVector::ZeroVector;
};


USTRUCT(BlueprintType)
struct FVaultInfo
{
	GENERATED_BODY()

	FVector StartPos = FVector::ZeroVector;
	FVector LandPos = FVector::ZeroVector;
	FVector RecoverPos = FVector::ZeroVector;

	FVaultVariant CurrentVaultVariant;
};


USTRUCT(BlueprintType)
struct FClimbInfo
{
	GENERATED_BODY()

	FVector StartPos = FVector::ZeroVector;
	FClimbVariant CurrentClimbVariant;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObstacleTraversalStartEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObstacleTraversalCompleteEvent);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class OBSTACLETRAVERSALPLUGIN_API UObstacleTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Determines whether to try to vault or climb first.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal")
		bool bTryVaultFirst = true;
	
	// The maximum distance an obstacle can be from the actor to be detected and traversed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal")
		float ObstacleQueryRange = 200;

	// The collision channel used to find obstacles to traverse.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal")
		TEnumAsByte<ECollisionChannel> ObstacleCollisionChannel = ECC_Visibility;

	// A list of heights at which raycasts will be used to find an obstacle. List in ascending order
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal")
		TArray<float> ObstacleQueryCastHeights;

	// The max angle between world move input and obstacle face inverse normal (towards obstacle) to traverse obstacle 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal")
		float MaxAngleToObstacleFaceNormal = 70;

	// The vertical difference between the characters feet and the character's position
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal")
	float CharacterFeetHeightOffset = -90;


	
	// The max depth of an obstacle in order to vault. Any longer, and the obstacle can only be climbed 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Vaulting")
		float MaxObstacleDepthToVault = 150;
	
	// The max depth of an obstacle in order to vault. Any longer, and the obstacle can only be climbed 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Vaulting")
		float VaultLandDistance = 100;
	
	// The range of valid height difference between landing position and starting position
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Vaulting")
		FVector2D VaultLandHeightRange = FVector2D(-50, 50);


	
	// Whether character can climb while in air/falling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Climbing")
		bool bCanClimbInAir = false;
	
	
	// A list of vault variants. List in ascending order of max obstacle height
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Vault Variants")
		TArray<FVaultVariant> VaultVariants;

	// A list of climb variants. List in ascending order of max obstacle height
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Climb Variants")
		TArray<FClimbVariant> ClimbVariants;
	

	
	// Controls whether debug drawings are shown for traces
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Debug")
		bool bShowDebug = false;

	// The duration which debug symbols (lines, spheres, etc.) are visible
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Obstacle Traversal|Debug")
		float DebugSymbolsLifeTime = 1;

	
	
	UObstacleTraversalComponent();

	// Public interface
	FObstacleTraversalStartEvent ObstacleTraversalStartEventDelegate;
	FObstacleTraversalCompleteEvent ObstacleTraversalCompleteEventDelegate;
	
	void BindInput(UInputComponent* PlayerInputComponent);


	// Driver function called upon traverse input
	void TryTraverse();
	
	void AllowTraversal();

	
	ETraversalType GetCurrentTraversalType();

	void OnClimbComplete();
	void OnVaultComplete();
	
	FVector GetTargetVaultPosition(EVaultState VaultState);
	FRotator GetTargetVaultRotation();

	FVector GetTargetClimbMountPosition();
	FRotator GetTargetClimbRotation();
	
protected:
	virtual void BeginPlay() override;

private:
	UWorld* World = nullptr;
	ACharacter* Character = nullptr;
	UCharacterMovementComponent* CharacterMovementComp = nullptr;
	UCapsuleComponent* CapsuleComp = nullptr;


	// State variables
	bool bCanTraverse = true;
	
	ETraversalType CurrentTraversalType = ETraversalType::TT_NONE;
	FObstacleInfo CurrentObstacleInfo;
	
	FVaultInfo CurrentVaultInfo;
	FClimbInfo CurrentClimbInfo;
	
	FVector CurrentCharFeetPos = FVector::ZeroVector;
	

	float MaxObstacleHeight = 0;

	FVector2D MoveInput = FVector2D::ZeroVector;
	FVector WorldMoveInput = FVector::ZeroVector;

	FCollisionQueryParams TraceCollisionQueryParams;
	
	
	// Returns if obstacle exists in direction of movement input found 
	bool FindObstacleTowardInputDirection();

	// Gathers information about any obstacle and writes to CurrentObstacleInfo
	// Returns false if unable to get any required info
	bool GetObstacleInfo();

	// Computes the obstacle DepthDirection and face normal.
	// Returns false if invalid direction
	bool GetObstacleDepthDirection();
	
	bool GetObstacleHeight();
	bool GetObstacleDepth();
	bool ComputeObstacleLedges();



	bool TryVault();
	bool TryClimb();

	void StartVault();
	void StartClimb();

	void StartTraversal(ETraversalType TraversalType);
	void CompleteTraversal(ETraversalType TraversalType);

	// Computes the maximum height of an obstacle to be valid for any traversal
	void ComputeMaxObstacleHeight();
	
	// Used to generate MoveInput
	void OnForwardInput(float InputValue);
	void OnRightInput(float InputValue);
	FVector GetWorldMoveInput();	// Returns the direction of movement input relative to player controller
};




	
