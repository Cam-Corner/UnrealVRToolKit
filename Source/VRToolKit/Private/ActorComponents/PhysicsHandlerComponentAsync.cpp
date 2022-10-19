#include "ActorComponents/PhysicsHandlerComponentAsync.h"
#include "ActorComponents/PhysicsHandlerComponent.h"

void PhysicsHandlerComponentAsync::OnPreSimulate_Internal()
{
	using namespace Chaos;

	const FPhysicsHandlerAsyncInput* Input = GetConsumerInput_Internal();

	if (!Input)
		return;


	if (Input->_PhysicsHandler.IsValid())
	{
		Input->_PhysicsHandler->AsyncTick(GetDeltaTime_Internal());
	}
}
