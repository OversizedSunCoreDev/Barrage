#include "BarrageDispatch.h"
#include "FWorldSimOwner.h"


UBarrageDispatch::UBarrageDispatch()
{
}

	UBarrageDispatch::~UBarrageDispatch() {
	}

	void UBarrageDispatch::Initialize(FSubsystemCollectionBase& Collection)
	{
		Super::Initialize(Collection);
	}

	void UBarrageDispatch::OnWorldBeginPlay(UWorld& InWorld)
	{
		Super::OnWorldBeginPlay(InWorld);
		JoltGameSim = MakeShareable(new FWorldSimOwner(TickRateInDelta));
	}

	void UBarrageDispatch::Deinitialize()
	{
		Super::Deinitialize();

	}

void UBarrageDispatch::SphereCast(double Radius, FVector3d CastFrom, double FinalRadius, uint64_t timestamp)
{
}

FBarrageKey UBarrageDispatch::CreateSimPrimitive(FBShapeParams& Definition)
{
	
	return FBarrageKey();
}

//unlike our other ecs components in artillery, barrage dispatch does not maintain the mappings directly.
//this is because we may have _many_ jolt sims running if we choose to do deterministic rollback in certain ways.
TSharedPtr<FBlet> UBarrageDispatch::GetShapeRef(FBarrageKey Existing)
{
	//JoltGameSim->BarrageToJoltMapping->FindChecked(Existing);
	return TSharedPtr<FBlet>();
}


TStatId UBarrageDispatch::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UBarrageDispatch, STATGROUP_Tickables);
}

void UBarrageDispatch::StepWorld()
{
	JoltGameSim->StepSimulation();
}