#pragma once
#include "Chaos/SimCallbackObject.h"
//#include "PhysicsHandlerComponentAsync.generated.h"

struct FPhysicsHandlerAsyncOutputData
{
	virtual ~FPhysicsHandlerAsyncOutputData() {}

};

struct FPhysicsHandlerAsyncOutput : public Chaos::FSimCallbackOutput
{
	FPhysicsHandlerAsyncOutput() : FSimCallbackOutput()
	{

	}

	void Reset() {}
};

struct FCachedPhysicsHandlerAsyncData
{
	void Validate(const FPhysicsHandlerAsyncOutput& Output) const
	{
		//esnure()
	}
};

//data that lives on the physics handler
struct VRTOOLKIT_API FPhysicsHandlerAsyncInputData
{
	virtual ~FPhysicsHandlerAsyncInputData() {}
};

//represents the update components state and implementation
struct VRTOOLKIT_API FUpdateComponentAsyncInput
{
	virtual ~FUpdateComponentAsyncInput() {}
};	

//this contains input from the GT (Game thread I think) that are applied to async sim state before simulation
struct FPhysicsHandlerGTInputs
{

};

/*
* Contains all input and implementation required to run async physics handler.
* Base implementation is from Physics handler.
* Contains 'PhysicsHandlerInput' and 'PhysicsHandlerInput' represent data/impl of Character and our UpdatedComponent.
* All input is const, non-const data goes in output. 'AsyncSimState' points to non-const sim state.
*/
struct VRTOOLKIT_API FPhysicsHandlerAsyncInput : public Chaos::FSimCallbackInput
{
	TWeakObjectPtr<class UPhysicsHandlerComponent> _PhysicsHandler;

	void Reset() 
	{
		_PhysicsHandler.Reset();
	}


};

class PhysicsHandlerComponentAsync : public Chaos::TSimCallbackObject<FPhysicsHandlerAsyncInput, FPhysicsHandlerAsyncOutput>
{

private:
	virtual void OnPreSimulate_Internal() override;
};