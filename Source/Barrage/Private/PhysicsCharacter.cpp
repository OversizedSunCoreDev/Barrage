﻿#include "PhysicsCharacter.h"

JPH::BodyID JOLT::FBCharacter::Create(CharacterVsCharacterCollision* CVCColliderSystem)
{
	BodyID ret = BodyID();
	// Create capsule
	if(World) 
	{
		//WorldSimOwner manages the lifecycle of the physics characters. we don't have a proper destructor in here yet, I'm just trying to get this UP for now.
		if(World)
		{
			Ref<Shape> capsule = new CapsuleShape(0.5f * mHeightStanding, mRadiusStanding);
			mCharacterSettings.mShape = RotatedTranslatedShapeSettings(
				Vec3(0, 0.5f * mHeightStanding + mRadiusStanding, 0), Quat::sIdentity(), capsule).Create().Get();
			// Configure supporting volume
			mCharacterSettings.mSupportingVolume = Plane(Vec3::sAxisY(), -mHeightStanding);
			// Accept contacts that touch the lower sphere of the capsule
			// Create character
			mCharacter = new CharacterVirtual(&mCharacterSettings, mInitialPosition, Quat::sIdentity(), 0, World.Get());
			//mCharacter->SetListener(this); 
			mCharacter->SetCharacterVsCharacterCollision(CVCColliderSystem); // see https://github.com/jrouwe/JoltPhysics/blob/e3ed3b1d33f3a0e7195fbac8b45b30f0a5c8a55b/UnitTests/Physics/CharacterVirtualTests.cpp#L759
			mEffectiveVelocity = Vec3::sZero();
			ret = mCharacter->GetInnerBodyID(); //I am going to regret this somehow.
		}
	}
	return ret;
}

void JOLT::FBCharacter::StepCharacter()
{
	// Determine new basic velocity
	Vec3 current_vertical_velocity = Vec3(0, mCharacter->GetLinearVelocity().GetY(), 0);
	Vec3 ground_velocity = mCharacter->GetGroundVelocity();
	Vec3 new_velocity = mForcesUpdate;
	mForcesUpdate = Vec3::sZero();
	//this ensures small forces won't knock us off the ground, vastly reducing jitter.
	if (mCharacter->GetGroundState() == CharacterVirtual::EGroundState::OnGround // If on ground
		&& (current_vertical_velocity.GetY() - ground_velocity.GetY()) < 0.3f)
	// And not moving away from ground
	{
		// Assume velocity of ground when on ground
		new_velocity += ground_velocity;
	}
	else
	{
		new_velocity += ground_velocity + current_vertical_velocity;
	}

	// Gravity
	if(World)
	{
		new_velocity += World->GetGravity() * mDeltaTime;
	}
	// Player input
	//todo: I think this is wrong now.
	new_velocity += mVelocityUpdate;

	// Update character velocity
	mCharacter->SetLinearVelocity(new_velocity);

	RVec3 start_pos = GetPosition();

	// Update the character position
	TempAllocatorMalloc allocator;
	mCharacter->ExtendedUpdate(mDeltaTime,
	                           World->GetGravity(),
	                           mUpdateSettings,
	                           World->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
	                           World->GetDefaultLayerFilter(Layers::MOVING),
	                           {},
	                           {},
	                           allocator);

	// Calculate effective velocity in this step
	mEffectiveVelocity = Vec3(GetPosition() - start_pos) / mDeltaTime;
}

void JOLT::FBCharacter::IngestUpdate(FBPhysicsInput& input)
{
	if (input.Action == PhysicsInputType::Rotation)
	{
		mCapsuleRotationUpdate = input.State;
	}
	else if (input.Action == PhysicsInputType::OtherForce)
	{
		mForcesUpdate += input.State.GetXYZ();
	}
	else if (input.Action == PhysicsInputType::Velocity || input.Action == PhysicsInputType::SelfMovement)
	{
		mVelocityUpdate += input.State.GetXYZ();
	}
			
}
