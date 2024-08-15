﻿#include "FBarragePrimitive.h"
#include "BarrageDispatch.h"
#include "FBPhysicsInput.h"
#include "CoordinateUtils.h"
#include "FWorldSimOwner.h"
//this is a long way to go to keep the types from leaking but I think it's probably worth it.

//don't add inline. don't do it!
FBarragePrimitive::~FBarragePrimitive()
{
	//THIS WILL NOT HAPPEN UNTIL THE TOMBSTONE HAS EXPIRED AND THE LAST SHARED POINTER TO THE PRIMITIVE IS RELEASED.
	//Only the CleanTombs function in dispatch actually releases the shared pointer on the dispatch side
	//but an actor might hold a shared pointer to the primitive that represents it after that primitive has been
	//popped out of this.
	if(GlobalBarrage != nullptr)
	{
		GlobalBarrage->FinalizeReleasePrimitive(KeyIntoBarrage);
	}
}

//-----------------
//the copy ops on call are intentional. they create a hold-open ref that allows us to be a little safer in many circumstances.
//once this thing is totally cleaned up, we can see about removing them and refactoring this.
//-----------------

void FBarragePrimitive::ApplyRotation(FQuat4d Rotator, FBLet Target)
{
	if(IsNotNull(Target))
	{
		if(GlobalBarrage)
		{
			if(MyBARRAGEIndex < ALLOWED_THREADS_FOR_BARRAGE_PHYSICS)
			{
				GlobalBarrage->ThreadAcc[MyBARRAGEIndex].Queue->Enqueue(
				FBPhysicsInput(Target, 0, PhysicsInputType::Rotation,
					CoordinateUtils::ToBarrageRotation(Rotator)
				)
				);
			}
		}
	}
}

//generally, this should be called from the same thread as update.
bool FBarragePrimitive::TryGetTransformFromJolt(FBLet Target, uint64 Time)
{
	if(IsNotNull(Target))
	{
		if(GlobalBarrage)
		{
			auto bID = GlobalBarrage->JoltGameSim->BarrageToJoltMapping->Find(Target->KeyIntoBarrage);
			if(bID)
			{
				if(GlobalBarrage->JoltGameSim->body_interface->IsActive(*bID))
				{
					
					//TODO: @Eliza, can we figure out if updating the transforms in place is threadsafe? that'd be vastly preferable
					//TODO: figure out how to make this less.... horrid.
					GlobalBarrage->GameTransformPump->Enqueue(UBarrageDispatch::TransformUpdate(
						Target->KeyOutOfBarrage,
						Time,
						CoordinateUtils::FromJoltCoordinates(GlobalBarrage->JoltGameSim->body_interface->GetLinearVelocity(*bID)),
						CoordinateUtils::FromJoltCoordinates(GlobalBarrage->JoltGameSim->body_interface->GetPosition(*bID)),
						CoordinateUtils::FromJoltRotation(GlobalBarrage->JoltGameSim->body_interface->GetRotation(*bID))
					));
					return true;
				}
			}
		}
		
	}
	return false;
}
FVector3d FBarragePrimitive::GetCentroidPossiblyStale(FBLet Target)
{
	//GlobalBarrage	
	if(IsNotNull(Target))
	{
		return FVector3d();
	}
	return FVector3d::ZeroVector;
}


void FBarragePrimitive::ApplyForce(FVector3d Force, FBLet Target)
{
	if(IsNotNull(Target))
	{
		if(GlobalBarrage)
		{
			if(MyBARRAGEIndex< ALLOWED_THREADS_FOR_BARRAGE_PHYSICS)
			{
				GlobalBarrage->ThreadAcc[MyBARRAGEIndex].Queue->Enqueue(
				FBPhysicsInput(Target, 0, PhysicsInputType::OtherForce,
					CoordinateUtils::ToBarrageForce(Force)
				)
				);
			}
		}
	}
}