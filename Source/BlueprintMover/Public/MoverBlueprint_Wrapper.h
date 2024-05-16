// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoveLibrary/ModularMovement.h"
#include "MoverDataModelTypes.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MoveLibrary/MovementRecord.h"
#include "MoveLibrary/MovementUtils.h"
#include "MoveLibrary/BasedMovementUtils.h"
#include "MoveLibrary/MoverBlackboard.h"
#include "MovementMode.h"
#include "MoverBlueprint_Wrapper.generated.h"

class UMoverComponent;
struct FMovementRecord;
struct FFloorCheckResult;
struct FOptionalFloorCheckResult;
struct FHitResult;
struct FProposedMove;

UCLASS()
class BLUEPRINTMOVER_API UMoverBlueprint_Wrapper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Mover)
	static bool TryMoveToStepUpWrap(USceneComponent* UpdatedComponent, UPrimitiveComponent* UpdatedPrimitive, UMoverComponent* MoverComponent, const FVector& GravDir, float MaxStepHeight, float MaxWalkSlopeCosine, float FloorSweepDistance, const FVector& MoveDelta, const FHitResult& Hit, const FFloorCheckResult& CurrentFloor, bool bIsFalling, FMovementRecord& MoveRecord);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static void FindFloorWrap(const USceneComponent* UpdatedComponent, const UPrimitiveComponent* UpdatedPrimitive, float FloorSweepDistance, float MaxWalkSlopeCosine, const FVector& Location, FFloorCheckResult& OutFloorResult);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static void ComputePenetrationAdjustmentWrap(const FHitResult& Hit, FVector& RequestedAdjustment);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static void TryMoveToResolvePenetrationWrap(USceneComponent* UpdatedComponent, UPrimitiveComponent* UpdatedPrimitive, /*EMoveComponentFlags MoveComponentFlags,*/ const FVector& ProposedAdjustment, const FHitResult& Hit, const FQuat& NewRotationQuat, FMovementRecord& MoveRecord);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static void TryMoveToAdjustHeightAboveFloorWrap(USceneComponent* UpdatedComponent, UPrimitiveComponent* UpdatedPrimitive, FFloorCheckResult& CurrentFloor, float MaxWalkSlopeCosine, FMovementRecord& MoveRecord);



	UFUNCTION(BlueprintCallable, Category = Mover)
	static bool GetMoveRecordRelevantVelocity(const FMovementRecord& MoveRecord, FVector& OutRelevantVelocity);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static void MoveRecord_SetDeltaSeconds(FMovementRecord& MoveRecord, const float DeltaSeconds);


	static FRelativeBaseInfo UpdateFloorAndBaseInfo(const FFloorCheckResult& FloorResult, UBaseMovementMode* Owner);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static void CaptureFinalState(USceneComponent* UpdatedComponent, UBaseMovementMode* Owner, bool bDidAttemptMovement, const FFloorCheckResult& FloorResult, const FMovementRecord& Record, FMoverDefaultSyncState& OutputSyncState);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static bool AttemptTeleport(USceneComponent* UpdatedComponent, UBaseMovementMode* Owner, const FVector& TeleportPos, const FRotator& TeleportRot, const FVector& PriorVelocity, FMoverTickEndData& Output);

	UFUNCTION(BlueprintCallable, Category = Mover)
	static bool AttemptJump(UCommonLegacyMovementSettings* CommonLegacySettings, float UpwardsSpeed, FMoverTickEndData& OutputState);
};
