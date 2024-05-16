// Fill out your copyright notice in the Description page of Project Settings.


#include "MoverBlueprint_Wrapper.h"

#include "MoveLibrary/GroundMovementUtils.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/MovementRecord.h"
#include "MoveLibrary/MovementUtils.h"
#include "MoveLibrary/BasedMovementUtils.h"
#include "MoveLibrary/MoverBlackboard.h"
#include "MovementMode.h"
#include "DefaultMovementSet/LayeredMoves/BasicLayeredMoves.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"


bool UMoverBlueprint_Wrapper::TryMoveToStepUpWrap(USceneComponent* UpdatedComponent, UPrimitiveComponent* UpdatedPrimimtive, UMoverComponent* MoverComponent, const FVector& GravDir, float MaxStepHeight, float MaxWalkSlopeCosine, float FloorSweepDistance, const FVector& MoveDelta, const FHitResult& Hit, const FFloorCheckResult& CurrentFloor, bool bIsFalling, FMovementRecord& MoveRecord)
{
	FOptionalFloorCheckResult OutStepDownResult;
	UGroundMovementUtils::TryMoveToStepUp(UpdatedComponent, UpdatedPrimimtive, MoverComponent, GravDir, MaxStepHeight, MaxWalkSlopeCosine, FloorSweepDistance, MoveDelta, Hit, CurrentFloor, bIsFalling, &OutStepDownResult, MoveRecord);
	return true;
}

void UMoverBlueprint_Wrapper::FindFloorWrap(const USceneComponent* UpdatedComponent, const UPrimitiveComponent* UpdatedPrimitive, float FloorSweepDistance, float MaxWalkSlopeCosine, const FVector& Location, FFloorCheckResult& OutFloorResult) {
	UFloorQueryUtils::FindFloor(UpdatedComponent, UpdatedPrimitive, FloorSweepDistance, MaxWalkSlopeCosine, Location, OutFloorResult);
}

void UMoverBlueprint_Wrapper::ComputePenetrationAdjustmentWrap(const FHitResult& Hit, FVector& RequestedAdjustment)
{
	RequestedAdjustment = UMovementUtils::ComputePenetrationAdjustment(Hit);
}

void UMoverBlueprint_Wrapper::TryMoveToResolvePenetrationWrap(USceneComponent* UpdatedComponent, UPrimitiveComponent* UpdatedPrimitive, /*EMoveComponentFlags MoveComponentFlags,*/ const FVector& ProposedAdjustment, const FHitResult& Hit, const FQuat& NewRotationQuat, FMovementRecord& MoveRecord)
{
	EMoveComponentFlags TempFlag = MOVECOMP_NoFlags;
	UMovementUtils::TryMoveToResolvePenetration(UpdatedComponent, UpdatedPrimitive, TempFlag, ProposedAdjustment, Hit, NewRotationQuat, MoveRecord);
}

void UMoverBlueprint_Wrapper::TryMoveToAdjustHeightAboveFloorWrap(USceneComponent* UpdatedComponent, UPrimitiveComponent* UpdatedPrimitive, FFloorCheckResult& CurrentFloor, float MaxWalkSlopeCosine, FMovementRecord& MoveRecord)
{
	UGroundMovementUtils::TryMoveToAdjustHeightAboveFloor(UpdatedComponent, UpdatedPrimitive, CurrentFloor, MaxWalkSlopeCosine, MoveRecord);
}



bool UMoverBlueprint_Wrapper::GetMoveRecordRelevantVelocity(const FMovementRecord& MoveRecord, FVector& OutRelevantVelocity)
{
	OutRelevantVelocity = MoveRecord.GetRelevantVelocity();
	return true;
}

void UMoverBlueprint_Wrapper::MoveRecord_SetDeltaSeconds(FMovementRecord& MoveRecord, const float DeltaSeconds)
{
	MoveRecord.SetDeltaSeconds(DeltaSeconds);
}


//Cringe function that has to be exposed
FRelativeBaseInfo UMoverBlueprint_Wrapper::UpdateFloorAndBaseInfo(const FFloorCheckResult& FloorResult, UBaseMovementMode* Owner)
{
	FRelativeBaseInfo ReturnBaseInfo;

	UMoverBlackboard* SimBlackboard = Owner->GetBlackboard_Mutable();

	SimBlackboard->Set(CommonBlackboard::LastFloorResult, FloorResult);

	if (FloorResult.IsWalkableFloor() && UBasedMovementUtils::IsADynamicBase(FloorResult.HitResult.GetComponent()))
	{
		ReturnBaseInfo.SetFromFloorResult(FloorResult);
	}

	return ReturnBaseInfo;
}



void UMoverBlueprint_Wrapper::CaptureFinalState(USceneComponent* UpdatedComponent, UBaseMovementMode* Owner, bool bDidAttemptMovement, const FFloorCheckResult& FloorResult, const FMovementRecord& Record, FMoverDefaultSyncState& OutputSyncState)
{
	FRelativeBaseInfo PriorBaseInfo;

	UMoverBlackboard* SimBlackboard = Owner->GetBlackboard_Mutable();

	const bool bHasPriorBaseInfo = SimBlackboard->TryGet(CommonBlackboard::LastFoundDynamicMovementBase, PriorBaseInfo);

	FRelativeBaseInfo CurrentBaseInfo = UpdateFloorAndBaseInfo(FloorResult, Owner);

	// If we're on a dynamic base and we're not trying to move, keep using the same relative actor location. This prevents slow relative 
	//  drifting that can occur from repeated floor sampling as the base moves through the world.
	if (CurrentBaseInfo.HasRelativeInfo()
		&& bHasPriorBaseInfo && !bDidAttemptMovement
		&& PriorBaseInfo.UsesSameBase(CurrentBaseInfo))
	{
		CurrentBaseInfo.ContactLocalPosition = PriorBaseInfo.ContactLocalPosition;
	}

	// TODO: Update Main/large movement record with substeps from our local record

	if (CurrentBaseInfo.HasRelativeInfo())
	{
		SimBlackboard->Set(CommonBlackboard::LastFoundDynamicMovementBase, CurrentBaseInfo);

		OutputSyncState.SetTransforms_WorldSpace(UpdatedComponent->GetComponentLocation(),
			UpdatedComponent->GetComponentRotation(),
			Record.GetRelevantVelocity(),
			CurrentBaseInfo.MovementBase.Get(), CurrentBaseInfo.BoneName);
	}
	else
	{
		SimBlackboard->Invalidate(CommonBlackboard::LastFoundDynamicMovementBase);

		OutputSyncState.SetTransforms_WorldSpace(UpdatedComponent->GetComponentLocation(),
			UpdatedComponent->GetComponentRotation(),
			Record.GetRelevantVelocity(),
			nullptr);	// no movement base
	}

	UpdatedComponent->ComponentVelocity = OutputSyncState.GetVelocity_WorldSpace();
}


bool UMoverBlueprint_Wrapper::AttemptTeleport(USceneComponent* UpdatedComponent, UBaseMovementMode* Owner, const FVector& TeleportPos, const FRotator& TeleportRot, const FVector& PriorVelocity, FMoverTickEndData& Output)
{
	if (UpdatedComponent->GetOwner()->TeleportTo(TeleportPos, TeleportRot))
	{
		FMoverDefaultSyncState& OutputSyncState = Output.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();

		OutputSyncState.SetTransforms_WorldSpace(UpdatedComponent->GetComponentLocation(),
			UpdatedComponent->GetComponentRotation(),
			PriorVelocity,
			nullptr); // no movement base

		// TODO: instead of invalidating it, consider checking for a floor. Possibly a dynamic base?
		if (UMoverBlackboard* SimBlackboard = Owner->GetBlackboard_Mutable())
		{
			SimBlackboard->Invalidate(CommonBlackboard::LastFloorResult);
			SimBlackboard->Invalidate(CommonBlackboard::LastFoundDynamicMovementBase);
		}

		return true;
	}

	return false;
}

bool UMoverBlueprint_Wrapper::AttemptJump(UCommonLegacyMovementSettings* CommonLegacySettings, float UpwardsSpeed, FMoverTickEndData& OutputState)
{
	// TODO: This should check if a jump is even allowed
	TSharedPtr<FLayeredMove_JumpImpulse> JumpMove = MakeShared<FLayeredMove_JumpImpulse>();
	JumpMove->UpwardsSpeed = UpwardsSpeed;
	OutputState.SyncState.LayeredMoves.QueueLayeredMove(JumpMove);
	OutputState.MovementEndState.NextModeName = CommonLegacySettings->AirMovementModeName;
	return true;
}